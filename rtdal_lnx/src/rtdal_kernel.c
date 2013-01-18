/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <libconfig.h>

#include "rtdal_kernel.h"
#include "str.h"
#include "defs.h"
#include "rtdal_time.h"
#include "rtdal_process.h"
#include "rtdal_context.h"
#include "rtdal_task.h"
#include "rtdal.h"
#include "rtdal_timer.h"
#include "futex.h"

#ifdef HAVE_UHD
#include "dac_cfg.h"
#include "uhd.h"
struct dac_cfg dac_cfg;
#endif

#define KERNEL_SIG_THREAD_SPECIFIC SIGRTMIN
#define N_THREAD_SPECIFIC_SIGNALS 6
const int thread_specific_signals[N_THREAD_SPECIFIC_SIGNALS] =
	{SIGSEGV, SIGBUS, SIGILL, SIGFPE, TASK_TERMINATION_SIGNAL, SIGUSR1};


int *core_mapping;
int nof_cores;

char libs_path[255];

static int using_uhd;
static rtdal_context_t rtdal;
static rtdal_timer_t kernel_timer;
static long int timeslot_us;
static enum clock_source clock_source;

static int sigwait_stops = 0;
static int multi_timer_futex;
static pid_t kernel_pid;
static pthread_t single_timer_thread;
static char UNUSED(sigmsg[1024]);
static int signal_received = 0;

static void go_out();
static void thread_signal_handler(int signum, siginfo_t *info, void *ctx);
static void print_license();

FILE *trace_buffer = NULL;
char *debug_trace_addr;
size_t debug_trace_sz;
FILE *debug_trace_file;


/**
 * A real-time fault has been detected. if rtdal.machine.rtFaultKill, calls
 * procThread.restoreThread() to kill the runningModule and restore the execution.
 * The process causing the real-time failure is removed from the ProcThread object
 * and a new thread is created beginning the execution with the first element in the pipeline.
 *
 * Set aerrorCode=RTFAULT value in module's Process object.
 *
 * *NOTE* that meanwhile, the module may be already finished and the next module
 *  in the pipeline will still receive an RT-fault. Indeed, when there is an
 *  rt-fault it is a system-wide problem, despite identifying which module is
 *  causing it is always helpful.
 */


inline static void kernel_tslot_run_rt_control() {
	if (rtdal.machine.rt_fault_opts == RT_FAULT_OPTS_HARD) {
		for (int i=0;i<rtdal.machine.nof_cores;i++) {
			hdebug("tslot=%d, pipeline=%d, ts_counter=%d, finished=%d\n",rtdal_time_slot(),
					rtdal.pipelines[i].id,rtdal.pipelines[i].ts_counter, rtdal.pipelines[i].finished);
			if (!rtdal.pipelines[i].finished
					&& rtdal.pipelines[i].running_process
					&& rtdal.pipelines[i].ts_counter < rtdal_time_slot()-1) {
				if (pipeline_rt_fault(&rtdal.pipelines[i])) {
					aerror("Couldn't kill pipeline after an rt-fault, "
							"going out\n");
				}
			}
		}
	}  else {
		aerror("Not implemented\n");
	}
}

static inline void kernel_tslot_run_periodic_callbacks() {
	/* Call periodic functions */
	for (int i=0;i<rtdal.nof_periodic;i++) {
		hdebug("function %d, counter %d\n",i,rtdal.periodic[i].counter);
		if (rtdal.periodic[i].counter==rtdal.periodic[i].period)  {
			hdebug("function %d, calling\n",i);
			rtdal.periodic[i].callback();
			rtdal.periodic[i].counter=0;
		}
		rtdal.periodic[i].counter++;
	}
}

inline void kernel_tslot_run() {
	rtdal_time_ts_inc();

	hdebug("tslot=%d\n",rtdal_time_slot());

	if (signal_received) {
		signal_received = 0;
	}

	kernel_tslot_run_rt_control();

	kernel_tslot_run_periodic_callbacks();
}

static int first_cycle = 0;
sem_t dac_sem;
/**
 * This function is called by the internal timer, a DAC event or by the sync_slave,
 * after the reception of a synchronization packet.
 */
static void kernel_cycle(void *x, struct timespec *time) {
	hdebug("now is %d:%d\n",time->tv_sec,time->tv_nsec);
	if (!first_cycle) {
		rtdal_time_reset_realtime(time);
		first_cycle = 1;
	}
	kernel_tslot_run();
	pipeline_sync_threads();
	if (clock_source == SINGLE_TIMER && using_uhd) {
		sem_post(&dac_sem);
	}
}

#ifdef HAVE_UHD
static void dac_cycle(void) {
	struct timespec time;
	if (clock_source == DAC) {
		clock_gettime(CLOCK_MONOTONIC,&time);
		kernel_cycle(NULL,&time);
	} else {
		if (!sigwait_stops) {
			sem_wait(&dac_sem);
		}
	}
}
#endif

inline static int kernel_initialize_setup_clock() {
	struct timespec start_time;
#ifdef HAVE_UHD
	struct sched_param param;
#endif

	/* access to kernel sync function is not allowed */
	rtdal.machine.slave_sync_kernel = NULL;

	switch(rtdal.machine.clock_source) {
	case  SINGLE_TIMER:
		kernel_timer.period_function = kernel_cycle;
		kernel_timer.period = rtdal.machine.ts_len_us*1000;
		kernel_timer.arg = NULL;
		kernel_timer.mode = NANOSLEEP;
		kernel_timer.wait_futex = NULL;
		kernel_timer.thread = &single_timer_thread;
		hdebug("creating single_timer_thread period %d\n",(int) kernel_timer.period);
		if (rtdal_task_new_thread(&single_timer_thread, timer_run_thread,
				&kernel_timer, DETACHABLE,
				rtdal.machine.kernel_prio-1, 0,0)) {
			rtdal_perror("rtdal_task_new_thread");
			return -1;
		}
		break;
	case MULTI_TIMER:
		usleep(100000);
		printf("Starting clocks in %d sec...\n", TIMER_FUTEX_GUARD_SEC);
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		for (int i=0;i<rtdal.machine.nof_cores;i++) {
			rtdal.pipelines[i].mytimer.next = start_time;
		}
		futex_wake(&multi_timer_futex);
		break;
	case DAC:
#ifdef HAVE_UHD
		param.sched_priority = rtdal.machine.kernel_prio-2;
		pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
		uhd_init(&dac_cfg, &rtdal.machine.ts_len_us,dac_cycle);
		param.sched_priority = rtdal.machine.kernel_prio;
		pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
#endif
		break;
	case SYNC_SLAVE:
		/* enable access to kernel_cycle function */
		rtdal.machine.slave_sync_kernel = kernel_cycle;
		break;
	default:
		aerror_msg("Unknown clock source %d\n", rtdal.machine.clock_source);
		return -1;
	}
	return 0;
}

int kernel_initialize_create_pipeline(pipeline_t *obj, int *wait_futex) {
	void *(*tmp_thread_fnc)(void*);
	void *tmp_thread_arg;

	hdebug("pipeline_id=%d\n",obj->id);
	if (rtdal.machine.clock_source == MULTI_TIMER) {
		obj->mytimer.period_function =
				pipeline_run_from_timer;
		obj->mytimer.period =
				rtdal.machine.ts_len_us*1000;
		obj->mytimer.arg = obj;
		obj->mytimer.wait_futex = wait_futex;
		obj->mytimer.mode = NANOSLEEP;

		obj->mytimer.thread =
				&obj->thread;
		tmp_thread_fnc = timer_run_thread;
		tmp_thread_arg = &obj->mytimer;
	} else {
		tmp_thread_fnc = pipeline_run_thread;
		tmp_thread_arg = obj;
	}

	if (rtdal_task_new_thread(&obj->thread,
			tmp_thread_fnc, tmp_thread_arg, DETACHABLE,
			rtdal.machine.kernel_prio-3, core_mapping[obj->id],0)) {
		rtdal_perror();
		return -1;
	}
	return 0;
}

/**
 * Creates the pipelines.
 * @return
 */
inline static int kernel_initialize_create_pipelines() {


	pipeline_initialize(rtdal.machine.nof_cores);
	hdebug("creating %d pipeline threads\n",rtdal.machine.nof_cores);

	for (int i=0;i<rtdal.machine.nof_cores;i++) {
		rtdal.pipelines[i].id = i;
		if (kernel_initialize_create_pipeline(&rtdal.pipelines[i],&multi_timer_futex)) {
			aerror("Creating pipeline\n");
			return -1;
		}
	}

	return 0;
}

/**
 * Block all signals except (SIGSEGV,SIGILL,SIGFPE,SIGBUS),
 * which sigwaited by the sigwait_thread
 */
inline static int kernel_initialize_setup_signals() {
	int s;
	sigset_t set;
	int i;
	struct sigaction action;

	action.sa_sigaction = thread_signal_handler;
	action.sa_flags = SA_SIGINFO;

        sigfillset(&set);
        /* do not block thread-specific signals. Add a handler to them */
        for (i=0;i<N_THREAD_SPECIFIC_SIGNALS;i++) {
                sigdelset(&set, thread_specific_signals[i]);
                if (sigaction(thread_specific_signals[i], &action, NULL)) {
                	poserror(errno, "sigaction");
                	return -1;
                }
        }
        s = sigprocmask(SIG_BLOCK, &set, NULL);
	if (s != 0) {
		poserror(errno, "sigprocmask");
		return -1;
	}
	return 0;
}

inline static int kernel_initialize_set_kernel_priority() {
	struct sched_param param;
	cpu_set_t cpuset;

	param.sched_priority = rtdal.machine.kernel_prio;
	hdebug("kernel_prio=%d\n",rtdal.machine.kernel_prio);

	if (sched_setscheduler(0, SCHED_FIFO, &param)) {
		if (errno == EPERM) {
			awarn("Not enough privileges to set kernel thread priority\n");
		} else {
			poserror(errno, "sched_setscheduler");
			return -1;
		}
	}

	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);

	if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset)) {
		poserror(errno, "sched_setaffinity");
		return -1;
	}

	return 0;
}

/**
 * Initializes the entire rtdal and kernel. This compromises:
 * a) initializing rtdal_base (parse platform file, create interfaces, etc.)
 * b) initialize node (configure and prepare interfaces, create processing
 * 	messages thread, etc.)
 * c) create a shared memory area for time sharing with the manager
 * 	(may be another process in the same processor)
 * c) setup signals
 * d) create different threads and assign priorities
 */
static int kernel_initialize(void) {

	/* Initialize rtdal_base library */
	rtdal.machine.clock_source = clock_source;
	rtdal.machine.ts_len_us = timeslot_us;
	rtdal.machine.kernel_prio = 50;
	rtdal.machine.rt_fault_opts = RT_FAULT_OPTS_HARD;
	rtdal_initialize_node(&rtdal, NULL, NULL);

	pthread_mutex_init(&rtdal.mutex,NULL);


	/* Set self priority to rtdal.machine.kernel_prio */
	if (kernel_initialize_set_kernel_priority()) {
		return -1;
	}

	/* Setup signals */
	if (kernel_initialize_setup_signals()) {
		return -1;
	}

	/* create pipelines */
	if (kernel_initialize_create_pipelines()) {
		return -1;
	}

	/* setup clock */
	if (kernel_initialize_setup_clock()) {
		return -1;
	}


	return 0;
}


/**
 * called after kernel timer caught a synchronous signal.
 * todo: try to recover thread?
 */
static void kernel_timer_recover_thread() {
	aerror("Not implemented\n");
}

static int sigwait_loop_process_thread_signal(siginfo_t *info) {
	int thread_id, signum;
	strdef(tmp_msg);

	signum = info->si_value.sival_int & 0xffff;
	thread_id = info->si_value.sival_int >> 16;
	hdebug("signum=%d, thread_id=%d\n",signum,thread_id);
	if (signum < N_THREAD_SPECIFIC_SIGNALS) {
		sprintf(tmp_msg, "Got signal num %d from ",
				thread_specific_signals[signum]);
	} else {
		sprintf(tmp_msg, "Got unknown signal from ");
	}

	/* now try to restore the pipeline, if the thread was a pipeline */
	if (thread_id > -1) {
		if (strlen(tmp_msg)>1) {
			sprintf(&tmp_msg[strlen(tmp_msg)-1],
					"pipeline thread %d process %d\n",
					thread_id,
					rtdal.pipelines[thread_id].running_process_idx);
		}
		if (pipeline_recover_thread(
				&rtdal.pipelines[thread_id])) {
			aerror("recovering pipeline thread\n");
		}

	} else if (info->si_value.sival_int == -1) {
		strcat(tmp_msg, "the kernel thread\n");
		kernel_timer_recover_thread();
	} else {
		strcat(tmp_msg, "an unkown thread\n");
	}
	return 1;
}

/**
 * This is a thread with priority kernel_prio that synchronously waits for
 * rtdal_pipeline signals (usign sigwaitinfo). All signals except thread-specific
 * ones (SIGSEGV,SIGILL,SIGBUS,SIGFPE) are blocked by all threads except this one.
 * Thread-specific signals are handled by ProcThreads which send a SIGRTMIN+1,
 * SIGRTMIN+2,SIGRTMIN+3,SIGRTMIN+4 (respectively) to this thread, which takes
 * actions accordingly.
 *
 * for signals SIGRTMIN to SIGRTMIN+4, cast the rtdal_pipeline object from this
 * si_value pointer and call rtdal_pipeline_recover_thread(pipeline,
 * pipeline->running_process, TRUE)
 */
static void sigwait_loop(void) {

	int signum;
	sigset_t set;
	siginfo_t info;

        sigfillset(&set);
        sigdelset(&set,TASK_TERMINATION_SIGNAL);
	while(!sigwait_stops) {
		do {
			signum = sigwaitinfo(&set, &info);
		} while (signum == -1 && errno == EINTR);
		if (signum == -1) {
			poserror(errno, "sigwaitinfo");
			goto out;
		}
		hdebug("detected signal %d\n",signum);
		if (signum == KERNEL_SIG_THREAD_SPECIFIC) {
			sigwait_loop_process_thread_signal(&info);
		} else if (signum == SIGINT) {
			printf("Caught SIGINT, exiting\n");
			fflush(stdout);
			goto out;
		} else if (signum != SIGWINCH && signum != SIGCHLD) {
			printf("Got signal %d, exiting\n", signum);
			fflush(stdout);
			goto out;
		}
	}

out:
	go_out();
}


/**
 * Handler for thread-specific signals (SIGSEGV,SIGILL,SIGFPE,SIGBUS).
 * Forwards a signal above SIGRTMIN to myself. Since it is blocked, it will
 * be received by sigwait_loop(), which is runs in the main kernel thread.
 *
 * The thread terminates after exiting the handler.
 */
static void thread_signal_handler(int signum, siginfo_t *info, void *ctx) {
	union sigval value;
	int thread_id;
	int i;

#ifdef PRINT_BT_ON_SIGSEGV
	void *pnt = NULL;
	ucontext_t *context = (ucontext_t*) ctx;
	char **msg;

	pnt = (void*) context->uc_mcontext.gregs[REG_EIP];
	msg = backtrace_symbols(&pnt, 1);
	strcat(sigmsg, msg[0]);
	free(msg);
#endif

	signal_received++;

	hdebug("[ts=%d] signal %d received\n",rtdal_time_slot(),signum);

	/* try to find the thread that caused the signal */

	/** todo: Caution!! is pthread_self() safe in the handler?
	 * it is not async-signal-safe by the standard,
	 * but the signals are synchronous.
	 */
	pthread_t thisthread = pthread_self();

	/* if signum is SIGUSR2, its a task termination signal, just exit */
	if (signum == TASK_TERMINATION_SIGNAL) {
		hdebug("sigusr2 signal. thread=%d\n",thisthread);
		goto cancel_and_exit;
	}

	/* is it a pipeline thread? */
	for (i=0;i<rtdal.machine.nof_cores;i++) {
		if (thisthread == rtdal.pipelines[i].thread) {
			break;
		}
	}
	if (i < rtdal.machine.nof_cores) {
		hdebug("pipeline_idx=%d\n",i);
		thread_id = i;

		/* set the thread to 0 because is terminating */
		rtdal.pipelines[thread_id].thread = 0;
	} else {
		/* it is not, may be it is the kernel timer */
		if (thisthread == single_timer_thread) {
			hdebug("timer thread=%d\n",thisthread);
			thread_id = -1;
		} else {
			/* @TODO: check if it is a status or init thread of any module */

			hdebug("other thread=%d\n",thisthread);
			goto cancel_and_exit;
		}
	}

	/* Now send a signal to the kernel */
	for (i=0;i<N_THREAD_SPECIFIC_SIGNALS;i++) {
		if (thread_specific_signals[i] == signum)
			break;
	}
	hdebug("signal=%d, thread=%d\n",i,thread_id);
	value.sival_int = thread_id<<16 | i;
	if (sigqueue(kernel_pid,
			KERNEL_SIG_THREAD_SPECIFIC,
			value)) {
		poserror(errno, "sigspscq");
	}

cancel_and_exit:
	pthread_exit(NULL);
}

static void check_threads() {
	int i;
	hdebug("nof_cores=%d, timer=%d\n",rtdal.machine.nof_cores,single_timer_thread);
	if (single_timer_thread) {
		if (!pthread_kill(single_timer_thread,0)) {
			aerror("kernel timer still running, killing\n");
			pthread_kill(single_timer_thread, TASK_TERMINATION_SIGNAL);
		}
	}
	for (i=0;i<rtdal.machine.nof_cores;i++) {
		hdebug("thread_%d=%d\n",i,rtdal.pipelines[i].thread);
		if (rtdal.pipelines[i].thread) {
			if (!pthread_kill(rtdal.pipelines[i].thread,0)) {
				aerror_msg("pipeline thread %d still running, killing\n",i);
				pthread_kill(rtdal.pipelines[i].thread, TASK_TERMINATION_SIGNAL);
			}
		}
	}
	hdebug("i=%d\n",i);
}


void open_debug_trace() {
	trace_buffer = open_memstream(&debug_trace_addr, &debug_trace_sz);
	if (!trace_buffer) {
		perror("opening debug_trace\n");
	}
	debug_trace_file = fopen("./out.trace","w");
	if (debug_trace_file) {
		perror("fopen");
	}
}

void write_debug_trace() {
	if (trace_buffer) {
		fclose(trace_buffer);
		trace_buffer = NULL;
	}
	if (debug_trace_addr == NULL) {
		return;
	}
	if (fwrite(debug_trace_addr,1,debug_trace_sz,debug_trace_file) == -1) {
		perror("fwrite");
	}
	debug_trace_addr = NULL;
}


static void go_out() {
	write_debug_trace();
	hdebug("tslot=%d\n",rtdal_time_slot());
	sigwait_stops = 1;
	sem_post(&dac_sem);
	kernel_timer.stop = 1;
	for (int i=0;i<rtdal.machine.nof_cores;i++) {
		if (rtdal.machine.clock_source == MULTI_TIMER) {
			rtdal.pipelines[i].mytimer.stop = 1;
		} else {
			rtdal.pipelines[i].stop = 1;
		}
	}
#ifdef HAVE_UHD
	if (using_uhd) {
		uhd_close();
	}
#endif
	usleep(100000);
	check_threads();
}


int parse_cores_comma_sep(char *str) {
	int i;
	size_t sz;
	char *tok;

	/* assume 10 cores */
	sz = 10;
	assert((core_mapping = malloc(sizeof(int)*sz)));
	i=0;
	tok = strtok(str,",");
	while (tok) {
		core_mapping[i] = atoi(tok);
		tok = strtok(0,",");
		i++;
		if ((size_t) i > sz) {
			sz += 10;
			assert((core_mapping = realloc(core_mapping, sizeof(int)*sz)));
		}
	}
	assert((core_mapping = realloc(core_mapping, sizeof(int)*(size_t)i)));

	return i;
}

int parse_cores_single_array(char *core_init, char *core_end) {
	int c_ini, c_end;
	if (core_init) {
		c_ini = atoi(core_init);
	} else {
		c_ini = 0;
	}
	if (core_end) {
		c_end = atoi(core_end);
	} else {
		return -1;
	}
	core_mapping = malloc(sizeof(int)*((size_t) c_end-(size_t)c_ini));
	for (int i=0;i<(c_end-c_ini);i++) {
		core_mapping[i] = i+c_ini;
	}
	return (c_end-c_ini);
}

/** Parses a string indicating which cores can be used to load modules
 * Valid string formats are:
 * - "N" Just a number, without "," nor ":" means to use core id 0 to N-1
 * - "n1:n2" Indicates that core ids n1 to n2 will be used
 * - "n1,n2,n3" Indicates that core ids n1, n2 and n3 only will be used
 */
int parse_cores(char *str) {
	char *dp;
	char *c;

	dp = index(str,':');
	c = index(str,',');

	if (!c && !dp) {
		return parse_cores_single_array(NULL,str);
	} else if (!c && dp) {
		*dp = '\0';
		dp++;
		return parse_cores_single_array(str,dp);
	} else if (c && !dp) {
		return parse_cores_comma_sep(str);
	} else {
		return -1;
	}
}

int parse_config(char *config_file) {
	config_t config;
	int ret = -1;
	config_setting_t *rtdal,*dac;
	const char *tmp;
	int single_timer;
	int time_slot_us;

	config_init(&config);
	if (!config_read_file(&config, config_file)) {
		aerror_msg("line %d - %s: \n", config_error_line(&config),
				config_error_text(&config));
		goto destroy;
	}

	rtdal = config_lookup(&config, "rtdal");
	if (!rtdal) {
		aerror("Error parsing config file: rtdal section not found.\n");
		goto destroy;
	}

	if (!config_setting_lookup_int(rtdal, "time_slot_us", &time_slot_us)) {
		aerror("time_slot_us field not defined\n");
		goto destroy;
	}

	if (!config_setting_lookup_string(rtdal, "cores", &tmp)) {
		aerror("cores field not defined\n");
		goto destroy;
	}
	nof_cores = parse_cores((char*) tmp);
	if (nof_cores < 0) {
		printf("Error invalid cores %s\n",tmp);
		exit(0);
	}

	if (!config_setting_lookup_bool(rtdal, "use_usrp", &using_uhd)) {
		aerror("use_usrp field not defined\n");
		goto destroy;
	}

	if (!config_setting_lookup_bool(rtdal, "timer_mode_single", &single_timer)) {
		aerror("timer_mode_single field not defined\n");
		goto destroy;
	}
	if (using_uhd) {
		if (single_timer) {
			clock_source = SINGLE_TIMER;
		} else {
			clock_source = DAC;
		}
	} else {
		if (single_timer) {
			clock_source = SINGLE_TIMER;
		} else {
			clock_source = MULTI_TIMER;
		}
	}
	if (!config_setting_lookup_string(rtdal, "path_to_libs", &tmp)) {
		aerror("path_to_libs field not defined\n");
		goto destroy;
	}
	strcpy(libs_path,tmp);

	if (using_uhd) {
		dac = config_lookup(&config, "dac");
		if (!dac) {
			aerror("Error parsing config file: dac section not found.\n");
			goto destroy;
		}

#ifdef HAVE_UHD
		double tmp;
		if (!config_setting_lookup_float(dac, "samp_freq", &dac_cfg.inputFreq)) {
			aerror("samp_freq field not defined\n");
			goto destroy;
		}
		dac_cfg.outputFreq = dac_cfg.inputFreq;

		if (!config_setting_lookup_float(dac, "rf_freq", &dac_cfg.inputRFFreq)) {
			aerror("rf_freq field not defined\n");
			goto destroy;
		}
		dac_cfg.outputRFFreq = dac_cfg.inputRFFreq;

		if (!config_setting_lookup_float(dac, "rf_gain", &tmp)) {
			aerror("rf_gain field not defined\n");
			goto destroy;
		}
		dac_cfg.tx_gain = tmp;
		dac_cfg.rx_gain = tmp;

		if (!config_setting_lookup_int(dac, "block_size", &dac_cfg.NsamplesIn)) {
			aerror("block_size field not defined\n");
			goto destroy;
		}
		dac_cfg.NsamplesOut = dac_cfg.NsamplesIn;

		if (!config_setting_lookup_bool(dac, "chain_is_tx", &dac_cfg.chain_is_tx)) {
			aerror("chain_is_tx field not defined\n");
			goto destroy;
		}

		dac_cfg.sampleType = 0;
		dac_cfg.nof_channels = 1;

		uhd_readcfg(&dac_cfg);
#endif
	}
	if (using_uhd) {
#ifdef HAVE_UHD
		timeslot_us = (long int) 1000000*((float) dac_cfg.NsamplesOut/dac_cfg.outputFreq);
#endif
	} else {
		timeslot_us = time_slot_us;
	}
	ret=0;
destroy:
	config_destroy(&config);
	return ret;
}

int main(int argc, char **argv) {

	if (argc!=3) {
		printf("Usage: %s path_to_waveform_model config_file\n",argv[0]);
		return -1;
	}

	if (parse_config(argv[2])) {
		aerror_msg("Error parsing file config %s\n",argv[2]);
		exit(0);
	}

#ifdef DEBUG_TRACE
	open_debug_trace();
	atexit(write_debug_trace);
#endif

	mlockall(MCL_CURRENT | MCL_FUTURE);

	kernel_pid = getpid();

	rtdal.machine.nof_cores = nof_cores;

	if (using_uhd) {
#ifdef HAVE_UHD
		struct sched_param param;
		sem_init(&dac_sem, 0, 0);
		param.sched_priority = rtdal.machine.kernel_prio-3;
		pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
		uhd_init(&dac_cfg, &rtdal.machine.ts_len_us,dac_cycle);
		param.sched_priority = rtdal.machine.kernel_prio;
		pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
#endif
	}

	print_license();
	if (getuid()) {
		printf("Run as root to run in real-time mode\n\n");
	}

	printf("Time slot:\t%d us\nPlatform:\t%d cores\nTimer:\t\t%s\n\n", (int) timeslot_us,
			rtdal.machine.nof_cores,(clock_source==MULTI_TIMER)?"Multi":"Single");

	/* initialize kernel */
	if (kernel_initialize()) {
		aerror("Initiating kernel\n");
		goto clean_and_exit;
	}

	if (rtdal_task_new(NULL,_run_main,argv[1])) {
		rtdal_perror("rtdal_task_new");
		goto clean_and_exit;
	}

	/* the main thread runs the sigwait loop */
	sigwait_loop();

clean_and_exit:
	exit(0);
}


static void print_license() {
	printf("%s  Copyright (C) %d http://flexnets.upc.edu/\n"
    "This program comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions. See license.txt\n\n",ALOE_VERSION, ALOE_YEAR);
}


/** @TODO MOVE THIS TO RTDAL_DAC
 *
 */
#ifdef HAVE_UHD


int rtdal_uhd_set_freq(float freq) {
	dac_cfg.outputFreq = (double) freq;
	return 0;
}
int rtdal_uhd_set_block_len(int len) {
	dac_cfg.NsamplesOut = len;
	return 0;
}
void *rtdal_uhd_buffer(int int_ch) {
	return dac_cfg.dacoutbuff[0];
}



#endif

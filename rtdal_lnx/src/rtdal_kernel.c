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


int *core_mapping;
int nof_cores;


int using_uhd;
rtdal_context_t rtdal;
static rtdal_timer_t kernel_timer;
long int timeslot_us;
enum clock_source clock_source;

int sigwait_stops = 0;
static int multi_timer_futex;
pid_t kernel_pid;
pthread_t single_timer_thread;
static char UNUSED(sigmsg[1024]);

static void print_license();

FILE *trace_buffer = NULL;
char *debug_trace_addr;
size_t debug_trace_sz;
FILE *debug_trace_file;

extern sem_t dac_sem;

/** Set timeslot to a multiple of the time slot defined platform-wide
*/
void rtdal_timeslot_set(int ts_base_multiply) {
        switch(rtdal.machine.clock_source) {
        case SINGLE_TIMER:
                kernel_timer.multiple = ts_base_multiply;
                break;
        case MULTI_TIMER:
                for (int i=0;i<rtdal.machine.nof_cores;i++) {
                        rtdal.pipelines[i].mytimer.multiple = ts_base_multiply;
                }
                break;
        default:
                aerror("Not implemented\n");
                break;
        }
}

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
		kernel_timer.multiple = 1;
#ifdef __XENO__
		kernel_timer.mode = XENOMAI;
#else
		kernel_timer.mode = NANOSLEEP;
#endif
		kernel_timer.wait_futex = NULL;
		kernel_timer.thread = &single_timer_thread;
		hdebug("creating single_timer_thread period %d\n",(int) kernel_timer.period);
		if (rtdal_task_new_thread(&single_timer_thread, timer_run_thread,
				&kernel_timer, DETACHABLE,
				rtdal.machine.kernel_prio, 0,0)) {
			rtdal_perror("rtdal_task_new_thread");
			return -1;
		}
		break;
	case MULTI_TIMER:
		usleep(100000);
		printf("Starting clocks in %d sec...\n", TIMER_FUTEX_GUARD_SEC);
		clock_gettime(CLOCK_REALTIME, &start_time);
		for (int i=0;i<rtdal.machine.nof_cores;i++) {
			rtdal.pipelines[i].mytimer.next = start_time;
			rtdal.pipelines[i].mytimer.multiple = 1;
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
	int prio;

	hdebug("pipeline_id=%d\n",obj->id);
	if (rtdal.machine.clock_source == MULTI_TIMER) {
		obj->mytimer.period_function =
				pipeline_run_from_timer;
		obj->mytimer.period =
				rtdal.machine.ts_len_us*1000;
		obj->mytimer.arg = obj;
		obj->mytimer.wait_futex = wait_futex;
#ifdef __XENO__
		obj->mytimer.mode = XENOMAI;
#else
		obj->mytimer.mode = NANOSLEEP;
#endif
		obj->mytimer.thread =
				&obj->thread;
		tmp_thread_fnc = timer_run_thread;
		tmp_thread_arg = &obj->mytimer;
		prio = rtdal.machine.kernel_prio;
	} else {
		tmp_thread_fnc = pipeline_run_thread;
		tmp_thread_arg = obj;
		prio = rtdal.machine.kernel_prio-1;
	}

	if (rtdal_task_new_thread(&obj->thread,
			tmp_thread_fnc, tmp_thread_arg, DETACHABLE,
			prio, core_mapping[obj->id],0)) {
		rtdal_perror();
		return -1;
	}
	return 0;
}

/**
 * Creates the pipelines.
 * @return
 */
static int kernel_initialize_create_pipelines() {


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


int kernel_initialize_set_kernel_priority() {
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
	rtdal.machine.kernel_prio = KERNEL_RT_PRIO;
	rtdal.machine.rt_fault_opts = RT_FAULT_OPTS_HARD;
	rtdal_initialize_node(&rtdal, NULL, NULL);

	pthread_mutex_init(&rtdal.mutex,NULL);


	/* Set self priority to rtdal.machine.kernel_prio */
#ifdef KERNEL_SIGWAIT_RT_PRIO
	if (kernel_initialize_set_kernel_priority()) {
		return -1;
	}
#endif

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


void kernel_exit() {
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

int main(int argc, char **argv) {

	mlockall(MCL_CURRENT | MCL_FUTURE);

	print_license();
	if (getuid()) {
		printf("Run as root to run in real-time mode\n\n");
	}

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

#ifdef __XENO__
	pthread_set_mode_np(0, PTHREAD_WARNSW);
#endif
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
int rtdal_uhd_get_block_len() {
	return dac_cfg.NsamplesIn;
}
void *rtdal_uhd_buffer(int ch) {
	if (ch) {
		return dac_cfg.dacoutbuff[0];
	} else {
		return dac_cfg.dacinbuff[0];
	}
}



#endif

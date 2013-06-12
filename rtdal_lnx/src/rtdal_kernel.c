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

#ifdef __XENO__
#include <rtdk.h>
#endif

#define HAVE_VOLK

#ifdef HAVE_VOLK
#include "volk/volk.h"
#endif

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
#include "barrier.h"
#include "pipeline_sync.h"

rtdal_context_t rtdal;
static rtdal_timer_t kernel_timer;

int sigwait_stops = 0;
static int multi_timer_futex;
pid_t kernel_pid;
pthread_t single_timer_thread,exec_timer_thread;
static char UNUSED(sigmsg[1024]);

static void print_license();

r_log_t rtdal_log,sched_log;

barrier_t start_barrier;

/** Set timeslot to a multiple of the time slot defined platform-wide
*/
void rtdal_timeslot_set(int ts_base_multiply) {
        switch(rtdal.machine.clock_mode) {
        case SINGLE_TIMER:
                kernel_timer.multiple = ts_base_multiply;
                break;
        case MULTI_TIMER:
                for (int i=0;i<rtdal.machine.nof_cores;i++) {
                        rtdal.pipelines[i].mytimer.multiple = ts_base_multiply;
                }
                break;
        default:
                break;
        }
}

void *exec_timer_none(void *arg) {
	while(1) {
		kernel_cycle(NULL,NULL);
	}
}

inline static int kernel_initialize_setup_clock() {
	struct timespec start_time;

	/* access to kernel sync function is not allowed */
	rtdal.machine.slave_sync_kernel = NULL;


	if (rtdal.machine.thread_sync_on_finish) {
		barrier_init(&start_barrier, rtdal.machine.nof_cores+1);
	}

	switch(rtdal.machine.clock_mode) {
	case  SINGLE_TIMER:
		kernel_timer.period_function = kernel_cycle;
		kernel_timer.period = rtdal.machine.ts_len_ns;
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

		if (rtdal.machine.logs_cfg.timing_en) {
			kernel_timer.log = rtdal_log_new("timer.time",UINT32,0);
			if (!kernel_timer.log) {
				aerror("Creating timer log\n");
				return -1;
			}
		}

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
	case NO_TIMER:
		if (rtdal_task_new_thread(&exec_timer_thread, exec_timer_none,
				NULL, DETACHABLE,
				rtdal.machine.kernel_prio, 0,0)) {
			rtdal_perror("rtdal_task_new_thread");
			return -1;
		}

		break;
	default:
		aerror_msg("Unknown clock source %d\n", rtdal.machine.clock_mode);
		return -1;
	}
	return 0;
}

int kernel_initialize_create_pipeline(pipeline_t *obj, int *wait_futex) {
	void *(*tmp_thread_fnc)(void*);
	void *tmp_thread_arg;
	int prio;

	hdebug("pipeline_id=%d\n",obj->id);
	if (rtdal.machine.clock_mode == MULTI_TIMER) {
		obj->mytimer.period_function =
				pipeline_run_from_timer;
		obj->mytimer.period =
				rtdal.machine.ts_len_ns;
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
	} else {
		tmp_thread_fnc = pipeline_run_thread;
		tmp_thread_arg = obj;
	}
	obj->wait_on_finish=rtdal.machine.thread_sync_on_finish;
	prio = rtdal.machine.kernel_prio-1;
	obj->xenomai_warn_msw = rtdal.machine.rt_cfg.xenomai_warn_msw;

	if (rtdal.machine.logs_cfg.timing_en) {
		char tmp[64];
		snprintf(tmp,64,"pipe_in_%d.time",obj->id);
		obj->log_in = rtdal_log_new(tmp,UINT32,0);
		if (!obj->log_in) {
			aerror_msg("Creating %s log\n",tmp);
			return -1;
		}
		snprintf(tmp,64,"pipe_out_%d.time",obj->id);
		obj->log_out = rtdal_log_new(tmp,UINT32,0);
		if (!obj->log_out) {
			aerror_msg("Creating %s log\n",tmp);
			return -1;
		}
		snprintf(tmp,64,"exec_%d.time",obj->id);
		obj->log_exec = rtdal_log_new(tmp,UINT32,0);
		if (!obj->log_exec) {
			aerror("Creating kernel execution log\n");
			return -1;
		}
	}

	if (rtdal_task_new_thread(&obj->thread,
			tmp_thread_fnc, tmp_thread_arg, DETACHABLE,
			prio, rtdal.machine.core_mapping[obj->id],0)) {
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


	rtdal_initialize_node(&rtdal, NULL, NULL);

	pthread_mutex_init(&rtdal.mutex,NULL);

	/* create logging service */
	if (rtdal.machine.logs_cfg.enabled) {
		if (rtdal_log_init(rtdal.machine.logs_cfg.base_path,200,4*1024,
				rtdal.machine.logs_cfg.log_length_mb*1024*1024,
				rtdal.machine.logs_cfg.log_to_stout?stdout:NULL)) {
			aerror("Creating logs\n");
			return -1;
		}
	}
	/* and create kernel log */
	if (rtdal.machine.logs_cfg.kernel_en) {
		rtdal_log = rtdal_log_new_opts("kernel.log",TEXT,0,RTDAL_LOG_OPTS_EXCL);
		if (!rtdal_log) {
			aerror("Creating kernel log\n");
			return -1;
		}
	}

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

	if (rtdal.machine.scheduling == SCHEDULING_PIPELINE) {
		/* create pipelines */
		if (kernel_initialize_create_pipelines()) {
			return -1;
		}

		/* setup clock */
		if (kernel_initialize_setup_clock()) {
			return -1;
		}
	}

	return 0;
}



static void check_threads() {
	int i;
	if (single_timer_thread) {
		if (!pthread_kill(single_timer_thread,0)) {
			aerror("kernel timer still running, killing\n");
			pthread_kill(single_timer_thread, TASK_TERMINATION_SIGNAL);
		}
	}
	for (i=0;i<rtdal.machine.nof_cores;i++) {
		if (rtdal.pipelines[i].thread) {
			if (!pthread_kill(rtdal.pipelines[i].thread,0)) {
				//aerror_msg("pipeline thread %d still running, killing\n",i);
				pthread_kill(rtdal.pipelines[i].thread, TASK_TERMINATION_SIGNAL);
			}
		} else {
			if (rtdal.pipelines[i].waiting) {
				rtdal.pipelines[i].waiting = 0;
				usleep(20000);
			}
		}
	}
}

void kernel_exit() {

	rtdal_log_flushall();

	sigwait_stops = 1;
	kernel_timer.stop = 1;
	for (int i=0;i<rtdal.machine.nof_cores;i++) {
		if (rtdal.machine.clock_mode == MULTI_TIMER) {
			rtdal.pipelines[i].mytimer.stop = 1;
		} else {
			rtdal.pipelines[i].stop = 1;
		}
	}
	usleep(100000);
	check_threads();
}
void *volk_malloc(int size) {
	void *ptr;
	int alignment = volk_get_alignment();
	if (posix_memalign(&ptr,alignment,size)) {
		return NULL;
	} else {
		return ptr;
	}
}
void volk_initialize() {
	void *x,*y,*z;
	float result;
	_Complex float hh=1;
	x=volk_malloc(128);
	y=volk_malloc(128);
	z=volk_malloc(128);
	volk_32fc_conjugate_32fc_a(x,y,16);
	volk_32fc_x2_multiply_32fc_a(x,y,z,16);
	volk_32fc_magnitude_32f_a(x,y,16);
	volk_32f_accumulator_s32f_a(&result,x,16);
	volk_32fc_s32fc_multiply_32fc_a(y,x,hh,16);
	volk_32fc_s32fc_multiply_32fc_u(y,x,hh,16);
	unsigned int target;
	volk_32f_index_max_16u_a(&target,x,16);
	free(x);
	free(y);
	free(z);
}

void load_volk() {
	printf("Loading VOLK library...\n");
	volk_initialize();
	printf("Done\n");
}

void print_schedinfo() {
	switch(rtdal.machine.scheduling) {
	case SCHEDULING_PIPELINE:
		printf("-- Pipeline Scheduling Selected --\n");
		printf("Time slot:\t%g us\nPlatform:\t%d cores\nTimer:\t\t", (float) rtdal.machine.ts_len_ns/1000,
				rtdal.machine.nof_cores);
		switch(rtdal.machine.clock_mode) {
		case SINGLE_TIMER:
			printf("Single\n\n");
			break;
		case MULTI_TIMER:
			printf("Multi\n\n");
			break;
		case NO_TIMER:
			printf("None\n\n");
		}
		break;
	case SCHEDULING_BESTEFFORT:
		printf("-- Best-Effort Scheduling Selected --\n");
	}
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

	if (parse_config(argv[2],&rtdal.machine)) {
		aerror_msg("Error parsing file config %s\n",argv[2]);
		exit(0);
	}

#ifdef HAVE_VOLK
	load_volk();
#endif

#ifdef __XENO__
	if (LOGS_ENABLED) {
		rt_print_auto_init(1);
	}
	pthread_set_mode_np(0, PTHREAD_WARNSW);
#endif
	kernel_pid = getpid();

	print_schedinfo();

	/* initialize kernel */
	if (kernel_initialize()) {
		aerror("Initiating kernel");
		goto clean_and_exit;
	}

	if (rtdal_task_new(NULL,_run_main,argv[1])) {
		rtdal_perror("rtdal_task_new");
		goto clean_and_exit;
	}

	/* the main thread runs the sigwait loop */
	sigwait_loop();

clean_and_exit:
	printf("exiting\n");
	kernel_exit();
	exit(0);
}


static void print_license() {
	printf("%s  Copyright (C) %d http://flexnets.upc.edu/\n"
    "This program comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions. See license.txt\n\n",ALOE_VERSION, ALOE_YEAR);
}




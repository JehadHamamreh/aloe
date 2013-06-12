#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <libconfig.h>
#include <assert.h>

#include <unistd.h>

#include "rtdal.h"
#include "rtdal_context.h"
#include "rtdal_kernel.h"
#include "pipeline.h"
#include "rtdal_time.h"

#include "barrier.h"
static int first_cycle = 0;
int signal_received = 0;

extern rtdal_context_t rtdal;

extern int sigwait_stops;

extern barrier_t start_barrier;


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

inline static int kernel_tslot_run_rt_control() {
	unsigned int has_exec[pipeline_MAX];
	int j=0,k=-1;

	for (int i=0;i<rtdal.machine.nof_cores;i++) {
		has_exec[i]=1;
		rtdal.pipelines[i].enable=1;
		hdebug("tslot=%d, pipeline=%d, ts_counter=%d, finished=%d\n",rtdal_time_slot(),
				rtdal.pipelines[i].id,rtdal.pipelines[i].ts_counter, rtdal.pipelines[i].finished);
		if (!rtdal.pipelines[i].finished) {
			rtdal.pipelines[i].finished=1;
			has_exec[i]=2;
			k=i;
			if (rtdal.machine.rt_cfg.exec_kill && rtdal.pipelines[i].running_process
					&& rtdal.pipelines[i].running_process->runnable) {
				rtdal.pipelines[i].running_process->finish_code = RTFAULT;
			}
		} else if (rtdal.pipelines[i].ts_counter < rtdal_time_slot()-1) {
			has_exec[i]=0;
			j++;
			if (rtdal.machine.rt_cfg.miss_kill && rtdal.pipelines[i].running_process
					&& rtdal.pipelines[i].running_process->runnable) {
				rtdal.pipelines[i].running_process->finish_code = RTFAULT;
			}
		}
		rtdal_log_add(rtdal.pipelines[i].log_exec,&has_exec[i],sizeof(unsigned int));
	}
	if (rtdal.machine.rt_cfg.miss_correct && j>0) {
		for (int i=0;i<rtdal.machine.nof_cores;i++) {
			if (has_exec[i]) {
				rtdal.pipelines[i].enable=0;
			}
		}
	}
	
	if (rtdal.machine.rt_cfg.exec_correct && k!=-1) {
		k=0;
		for (int i=0;i<rtdal.machine.nof_cores;i++) {
			if (has_exec[i] != 2) {
				k++;
			}
			rtdal.pipelines[i].ts_counter = rtdal_time_slot();
		}
		if (k) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
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

inline int kernel_tslot_run() {
	rtdal_time_ts_inc();

	hdebug("tslot=%d\n",rtdal_time_slot());

	if (signal_received) {
		signal_received = 0;
	}

	if (rtdal.machine.rt_cfg.do_rtcontrol) {
		if (kernel_tslot_run_rt_control()) {
			return 0;
		}
	}

	for (int i=0;i<rtdal.machine.nof_cores;i++) {
		rtdal.pipelines[i].enable=1;
	}

	kernel_tslot_run_periodic_callbacks();
	return 1;
}

/**
 * This function is called by the internal timer, a DAC event or by the sync_slave,
 * after the reception of a synchronization packet.
 */
void kernel_cycle(void *x, struct timespec *time) {
	struct timespec t;
	if (!first_cycle) {
		if (!time) {
			clock_gettime(CLOCK_REALTIME,&t);
			time=&t;
		}
		rtdal_time_reset_realtime(time);
		first_cycle = 1;
	}
	if (kernel_tslot_run()) {
		pipeline_sync_threads();
	}
	if (rtdal.machine.thread_sync_on_finish) {
		barrier_wait(&start_barrier);
	}
}


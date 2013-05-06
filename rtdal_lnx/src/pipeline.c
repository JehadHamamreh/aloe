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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <ucontext.h>
#include <execinfo.h>
#include <sys/ucontext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pipeline.h"
#include "rtdal.h"
#include "rtdal_error.h"
#include "rtdal_time.h"
#include "rtdal_kernel.h"
#include "pipeline_sync.h"
#include "defs.h"

#define NSEC_DEB_LEN 120000
struct timespec nsec_deb[NSEC_DEB_LEN];
struct timespec nsec_deb2[NSEC_DEB_LEN];

static int timer_first_cycle = 0;
static int num_pipelines;
static int is_first_in_cycle_count;

extern int timeslot_p[MAX_PIPELINES];

int pgroup_notified_failure[MAX_PROCESS_GROUP_ID];

void pipeline_initialize(int _num_pipelines) {
	hdebug("num_pipelines=%d\n",_num_pipelines);
	num_pipelines = _num_pipelines;
	pipeline_sync_initialize(num_pipelines);
}

inline void pipeline_sync_thread_idx(int idx) {
	hdebug("syncing\n",0);
	pipeline_sync_threads_wake_idx(idx);
}

inline void pipeline_sync_threads() {
	hdebug("syncing\n",0);
	pipeline_sync_threads_wake();
}

inline static void pipeline_run_thread_run_module(pipeline_t *pipe,
		rtdal_process_t *proc, int idx) {
	hdebug("pipeid=%d, pid=%d, idx=%d, run=%d\n",pipe->id,proc->pid,idx,proc->runnable);
	if (proc->runnable) {
		pipe->running_process = proc;
		pipe->running_process_idx = idx;
		proc->is_running = 1;
		if (proc->run_point(proc->arg)) {
			aerror_msg("Error running module %d:%d\n",
					pipe->id,pipe->running_process_idx);
			pipeline_remove(pipe,proc);
			proc->is_running = 0;
		}
		proc->is_running = 0;
	}
}


inline static void pipeline_run_thread_check_status(pipeline_t *pipe,
		rtdal_process_t *proc) {

	hdebug("pipeid=%d, pid=%d, run=%d, code=%d, waveform_notify=%d\n",pipe->id,proc->pid,
			proc->runnable,proc->finish_code, pgroup_notified_failure[proc->attributes.process_group_id]);
	if (proc->runnable && proc->finish_code != FINISH_OK &&
			!pgroup_notified_failure[proc->attributes.process_group_id]) {
		if (proc->attributes.finish_callback) {
			hdebug("calling finish 0x%x arg=0x%x\n",proc->attributes.finish_callback,
					proc->arg);
			printf("fault-%d\n",rtdal_time_slot());
			pgroup_notified_failure[proc->attributes.process_group_id] = 1;
			rtdal_task_new(NULL, proc->attributes.finish_callback,proc->arg);
		} else {
			aerror_msg("Abnormal pid=%d termination but no callback was defined\n",
					proc->pid);
		}
	}
}

inline static void pipeline_run_thread_print_time(pipeline_t *obj) {
#ifdef PRINT_TIME
	time_t tdata;
	rtdal_time_get(&tdata);
	if (!(obj->ts_counter%10000))
		printf("Pipeline %d running %d modules at TS=%d. RTF=%d\t Start: %d:%d\n",obj->id, obj->nof_processes,
				rtdal_time_slot(),obj->rtfaults,
				(int) tdata.tv_sec, (int) tdata.tv_usec);
#endif

}
/**
 * Atomic modulo num_pipelines counter.
 */
inline static int is_first_in_cycle() {
	return __sync_fetch_and_add(&is_first_in_cycle_count, 1)
			& (num_pipelines - 1);
}

inline static void pipeline_run_time_slot(pipeline_t *obj, struct timespec *time) {
	int idx;
	rtdal_process_t *run_proc;
	hdebug("pipeid=%d, tslot=%d, nof_process=%d thread=%d\n",obj->id,obj->ts_counter,
			obj->nof_processes, obj->thread);
	obj->finished = 0;

	pipeline_run_thread_print_time(obj);

	run_proc = obj->first_process;
	idx = 0;

	timelog(obj->log_in);

	if (obj->enable) {
		while(run_proc) {
			hdebug("%d/%d: run=%d code=%d next=0x%x\n",idx,obj->nof_processes,run_proc->runnable,
					run_proc->finish_code,run_proc->next);
			if (idx > obj->nof_processes) {
				aerror_msg("Fatal error. Corrupted pipeline-%d process list at process %d\n",
						obj->id, idx);
				kill(getpid(),SIGTERM);
				pthread_exit(NULL);
			}
			pipeline_run_thread_check_status(obj,run_proc);
			pipeline_run_thread_run_module(obj,run_proc, idx);
			run_proc = run_proc->next;
			idx++;
		}
	}

	timelog(obj->log_out);

	obj->ts_counter=rtdal_time_slot();

	obj->finished = 1;
}



void pipeline_run_from_timer(void *arg, struct timespec *time) {
	pipeline_t *obj = (pipeline_t*) arg;

	hdebug("now is %d:%d\n",time->tv_sec,time->tv_nsec);

	if (!timer_first_cycle && time) {
		rtdal_time_reset_realtime(time);
		timer_first_cycle = 1;
	}

	if (!is_first_in_cycle()) {
		kernel_tslot_run();
	}

	pipeline_run_time_slot(obj,time);
}

/**
 * Runs one process after another (calling process[i].run_point()) and then
 * sleeps waiting for the semaphore
 */
void *pipeline_run_thread(void *self) {
	assert(self);
	pipeline_t *obj = (pipeline_t*) self;
	assert(obj->id>=0);

#ifdef __XENO__
	pthread_set_mode_np(0, PTHREAD_WARNSW);
#endif

	hdebug("pipeid=%d waiting\n",obj->id);

	obj->stop = 0;
	pipeline_sync_thread_waits(obj->id);
	hdebug("pipeid=%d start\n",obj->id);
	while(!obj->stop) {
		pipeline_run_time_slot(obj, NULL);
		pipeline_sync_thread_waits(obj->id);
	}
	hdebug("pipeid=%d exiting\n",obj->id);
	return NULL;
}

/**  Called from the sigwait kernel thread after a pipeline thread has died.
 * Set the process that caused the fault as non-runnable and create a new pipeline thread.
 */
int pipeline_recover_thread(pipeline_t *obj) {
	hdebug("pipeline_id=%d\n",obj->id);
	obj->finished = 1;
	if (rtdal_process_seterror((r_proc_t) obj->running_process, SIG_RECV)) {
		aerror("setting process error\n");
		return -1;
	}
/*
	if (kernel_initialize_create_pipeline(obj, NULL)) {
		aerror("creating pipeline thread\n");
		return -1;
	}
*/	return 0;
}

/**  Called when there is an rtfault in the pipeline
 */
int pipeline_rt_fault(pipeline_t *obj) {
#ifdef KILL_RT_FAULT
	hdebug("pipeline_id=%d, process_idx=%d\n",obj->id,obj->running_process_idx,obj->running_process_idx);
	obj->finished = 1;
	aerror_msg("+++[ts=%d]+++ RT-Fault detected at pipeline %d, process %d\n",obj->ts_counter,obj->id,
			obj->running_process_idx);
	if (obj->thread) {
		int s = pthread_kill(obj->thread,SIGUSR1);
		if (s) {
			return -1;
		}
	}
#else
	obj->finished = 1;
	obj->rtfaults++;
/*	if (obj->running_process->runnable) {
		obj->running_process->finish_code = RTFAULT;
	}
*/
#ifdef PRINT_RT_FAULT
	printf("+++[ts=%d]+++ RT-Fault: Process %d/%d still running in pipeline %d\n",
			obj->ts_counter, obj->running_process_idx, obj->nof_processes, obj->id);
#else
	char tmp[2];
	snprintf(tmp,2,"%d",obj->id);
	write(0,tmp,strlen(tmp));
#endif
#endif
	return 0;
}


/**
 * Adds a process to the pipeline. It is inserted in the position
 * min(n,exec_position) where n is the number of
 * elements in the spscq and exec_position is defined in the process attributes
 * used in the call to rtdal_process_new().
 * Returns the position it has finally been inserted.
 *
 *  @param obj Pointer to the pipeline_t object where the process is inserted
 *  @param process pointer to the rtdal_process_t object to insert.
 *  @returns non-negative integer number indicating the position it has been
 *  inserted, or -1 on error.
 */
int pipeline_add(pipeline_t *obj, rtdal_process_t *process) {
	hdebug("pipeid=%d, nof_process=%d, pid=%d, exec_pos=%d\n",obj->id,obj->nof_processes,
			process->pid,process->attributes.exec_position);
	RTDAL_ASSERT_PARAM(obj);
	RTDAL_ASSERT_PARAM(process);
	int exec_pos, i=0;
	rtdal_process_t *p = NULL;

	exec_pos = process->attributes.exec_position;

	/* head because empty list */
	if (!obj->first_process) {
		hdebug("pipeid=%d add pid=%d to head\n", obj->id, process->pid);
		obj->first_process = process;
		process->next = NULL;
		goto end;
	}
	/* head because first exec position */
	if (exec_pos < obj->first_process->attributes.exec_position) {
		hdebug("pipeid=%d add pid=%d to head\n", obj->id, process->pid);
		process->next = obj->first_process;
		obj->first_process = process;
		goto end;
	}
	/* middle */
	i=0;
	p = obj->first_process;
	while(p->next && exec_pos >= p->next->attributes.exec_position) {
		p=p->next;
		i++;
	}
	process->next = p->next;
	p->next = process;

	hdebug("pipeid=%d, add pid=%d to pos=%d\n", obj->id, process->pid,i);

end:
	obj->nof_processes++;
	/* assign pipeline to object */
	process->pipeline = obj;

	return i;
}

/**
 * Removes the process proc from the pipeline pointed by obj.
 * @param obj Pointer to the pipeline object
 * @param proc Pointer to the process to remove
 * @return Zero on success, -1 on error
 */
int pipeline_remove(pipeline_t *obj, rtdal_process_t *proc) {
	hdebug("pipeid=%d, nof_process=%d, pid=%d, pid_pos=%d\n",obj->id,obj->nof_processes,
			proc->pid,proc->attributes.exec_position);
	RTDAL_ASSERT_PARAM(obj);
	RTDAL_ASSERT_PARAM(proc);

	rtdal_process_t *cur, *prev;

	prev = NULL;
	cur = obj->first_process;
	while(cur != proc && cur) {
		hdebug("pipeid=%d, prev=0x%x, cur=0x%x\n", obj->id,
				prev,cur);
		prev = cur;
		cur = cur->next;
	}
	if (!cur) {
		RTDAL_SETERROR(RTDAL_ERROR_NOTFOUND);
		return -1;
	}
	if (prev) {
		hdebug("pipeid=%d remove middle/end\n",obj->id);
		prev->next = cur->next;
	} else {
		hdebug("pipeid=%d remove first\n",obj->id);
		obj->first_process = cur->next;
	}

	obj->nof_processes--;
	proc->next = NULL;

	return 0;
}




#include <stdlib.h>
#include <unistd.h>

#include "rtdal.h"
#include "rtdal_context.h"
#include "rtdal_task.h"
#include "rtdal_process.h"
#include "pipeline.h"
#include "defs.h"
#include "modulethread.h"

#define WAIT_TOUT_US	500000
extern rtdal_context_t rtdal;

extern int pgroup_notified_failure[MAX_PROCESS_GROUP_ID];



inline static void run_check_status(rtdal_process_t *proc) {

	if (proc->runnable && proc->finish_code != FINISH_OK &&
			!pgroup_notified_failure[proc->attributes.process_group_id]) {
		if (proc->attributes.finish_callback) {
			hdebug("calling finish 0x%x arg=0x%x\n",proc->attributes.finish_callback,
					proc->arg);
			pgroup_notified_failure[proc->attributes.process_group_id] = 1;
			proc->attributes.finish_callback(proc->arg);
		} else {
			aerror_msg("Abnormal pid=%d termination but no callback was defined\n",
					proc->pid);
		}
	}
}


void *modulethread_run(void *arg) {
	rtdal_process_t *proc = (rtdal_process_t*) arg;

	while(1) {
		while(!proc->runnable)
			sleep(1);

		proc->is_running = 1;
		if (proc->run_point) {
			if (proc->run_point(proc->arg)) {
				aerror_msg("Error running module %s\n",proc->attributes.binary_path);
				modulethread_remove(proc);
				proc->is_running = 0;
				return NULL;
			}
		} else {
			return NULL;
		}
		proc->is_running = 0;
		run_check_status(proc);
	}
	return NULL;
}

int modulethread_new(rtdal_process_t *proc) {
	return rtdal_task_new_prio(NULL, modulethread_run, proc, rtdal.machine.kernel_prio-1, -1);
}

int modulethread_remove(rtdal_process_t *proc) {
	proc->run_point = NULL;
	return 0;
}

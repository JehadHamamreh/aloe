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

#include <stddef.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#ifdef __XENO__
#include <rtdk.h>
#endif

#include "rtdal.h"
#include "rtdal_kernel.h"
#include "objects_max.h"
#include "rtdal_context.h"
#include "rtdal_time.h"
#include "modulethread.h"
#include "rtdal_file.h"
#include "rtdal_error.h"
#include "rtdal_itfspscq.h"

#include "defs.h"
#include "str.h"

static rtdal_context_t *context;
extern int pgroup_notified_failure[MAX_PROCESS_GROUP_ID];

void rtdal_printf(const char *format, ...) {
	va_list ap;
	va_start(ap,format);
#ifdef __XENO__
	rt_vprintf(format,ap);
#else
	vprintf(format,ap);
#endif
}

/** Enables/Disables real-time control 
*/
void rtdal_rtcontrol_enable(int enabled) {
	context->machine.rt_fault_opts = enabled?RT_FAULT_OPTS_HARD:0; 
}


/** Writes the machine structure into the buffer pointed by *machine
 *
 */
void rtdal_machine(rtdal_machine_t *machine) {
	assert(context);

	memcpy(machine,&context->machine,sizeof(rtdal_machine_t));
}

/**
 * Must be called before any other function.
 *
 * 1) Parse configuration file and fill +machine attributes
 * 2) Create interfaces. Call -createNodeInterfaces()
 * 3) Setup synchronization:
 *   3.1) If I am the synchronization master, set machine.clockSource=CLOCKSRC_TIMER if source="timer" or machine.clockSource=CLOCKSRC_DAC if source="dac:/"
 *   3.2) If I am an slave node, set machine.clockSource=CLOCKSRC_TIMER if continuous_mode=FALSE or machine.clockSource=CLOCKSRC_SYNC if continuous_mode=TRUE
 *   3.3) Set machine.syncContinuous boolean if continuous_mode=TRUE
 *   3.4) Set machine.syncMaster boolean if I am the synchronization master reference
 *   3.5) Set machine.syncPeriod to the synchronization period
 * 4) setup DAC (if my node has a DAC). For each DAC, create an instance of the DAC type and:
 *   4.1) Call DAC.openDAC(address,options,null) if machine.clockSource!=CLOCKSRC_DAC
 *   4.2) Call DAC.setScheduler(tsBeginFnc, machine.kernelPrio) if machine.clockSource==CLOCKSRC_DAC
 * 5) create max_spscqs_x_node +spscqs and max_modules_x_node +procs objects
 */
int rtdal_initialize_node(rtdal_context_t *_context, string config_file,
		void (*ts_begin_fnc)(void) ) {
	assert(_context);

	context = _context;

	rtdal_error_set_context(&context->error);
	context->time.ts_len_us = context->machine.ts_len_ns/1000;
	rtdal_time_set_context(&context->time);

	rtdal_time_reset();

	return -1;
}


/**
 * 1) PhysicItf.create("tcp://local_ip:ctrl_port") mode in name "ctrl"
 * 2) PhysicItf.create("udp://local_ip:sync_port") mode in name "sync"
 * 3) PhysicItf.create("udp://local_ip:probe_port") mode in name "probe"
 * 4) If !multicast, PhysicItf.create("udp://node_ip:sync_port") mode out name "slavesync%d" for each n in node list
 * 5) If multicast, PhysicItf.create("udp://MULTICAST_GROUP1:sync_port") mode out name "slavesync0" (only one output interface is created)
 * 6) Create data interfaces with name "data_id" with id consecutive numbers
 */
static int __attribute__((__unused__)) start_node_interfaces(string config_file) {
	assert(context);
	aerror("Not yet implemented");
	return -1;
}

/**
 * Configures the control, probe and sync interfaces for the manager
 * 1) For each node, create an interface with name ctrl_nodename
 * 2) For each node, create an interface with name probe_nodename
 * 3) if clockLocal create the sync interface to the sync master, with name "sync"
 */
int rtdal_start_manager_interfaces(string config_file) {
	assert(context);
	aerror("Not yet implemented");
	return -1;
}







/**
 * Creates a new low-priority periodic function. If it succeeds, the function callback
 * will be called every period timeslots with low priority.
 *
 * @param callback Pointer to the periodic function
 * @param period Positive integer, in time slots
 * @return zero on success, -1 on error
 */
int rtdal_periodic_add(void (*callback)(void), int period) {
	assert(context);
	RTDAL_ASSERT_PARAM(callback);
	RTDAL_ASSERT_PARAM(period>0);

	pthread_mutex_lock(&context->mutex);

	int i;

	for (i=0;i<MAX(rtdal_periodic);i++) {
		if (!context->periodic[i].callback)
			break;
	}
	if (i == MAX(rtdal_periodic)) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		pthread_mutex_unlock(&context->mutex);
		return -1;
	}
	context->periodic[i].counter = 0;
	context->periodic[i].period = period;
	context->periodic[i].callback = callback;
	pthread_mutex_unlock(&context->mutex);
	hdebug("i=%d, period=%d, callback=0x%x\n",i,period,callback);
	return 0;
}

/**
 * Removes the function pointed by callback from the kernel periodic callback functions,
 * previously added with rtdal_periodic_add().
 *
 * @param callback Pointer to the periodic function
 * @returns zero on success, -1 on error
 */
int rtdal_periodic_remove(void (*callback)(void)) {
	assert(context);
	RTDAL_ASSERT_PARAM(callback);

	int i;
	pthread_mutex_lock(&context->mutex);
	for (i=0;i<MAX(rtdal_periodic);i++) {
		if (context->periodic[i].callback == callback)
			break;
	}

	if (i == MAX(rtdal_periodic)) {
		RTDAL_SETERROR(RTDAL_ERROR_NOTFOUND);
		return -1;
	}
	context->periodic[i].counter = 0;
	context->periodic[i].period = 0;
	context->periodic[i].callback = NULL;
	pthread_mutex_unlock(&context->mutex);
	hdebug("i=%d\n",i);
	return 0;
}

/**  sleep the calling thread for the time specified by the time_t structure.
 */
int rtdal_sleep(time_t *t) {
	assert(context);
	RTDAL_ASSERT_PARAM(t);
	hdebug("sleep_for=%d:%d\n",t->tv_sec,t->tv_usec);
	struct timespec sleep;

	sleep.tv_sec = t->tv_sec;
	sleep.tv_nsec = t->tv_usec*1000;

	if (clock_nanosleep(CLOCK_REALTIME,0,&sleep,NULL)) {
		RTDAL_SYSERROR("clock_nanosleep");
		return -1;
	}

	hdebug("waking up at %d\n",rtdal_time_slot());
	return 0;
}

/**
 * Returns a handler to the first physical interface matching the name.
 * @param name Name of the physical interface
 * @return non-null value on success, zero on error
 */
r_itf_t rtdal_itfphysic_get(string name) {
	hdebug("name=%s\n",name);
	assert(context);
	RTDAL_ASSERT_PARAM_P(name);

	int i;
	for (i=0;i<MAX(rtdal_itfphysic);i++) {
		if (!strcmp(context->physic_itfs[i].parent.name, name))
			break;
	}
	hdebug("i=%d\n",i);

	if (i == MAX(rtdal_itfphysic)) {
		RTDAL_SETERROR(RTDAL_ERROR_NOTFOUND);
		return NULL;
	}
	return (r_itf_t) &context->physic_itfs[i];
}

/**
 * Returns a handler to the first physical interface matching the id.
 * @param id Id of the physical interface
 * @return non-null value on success, zero on error
 */
r_itf_t rtdal_itfphysic_get_id(int id) {
	hdebug("id=%d\n",id);
	assert(context);
	RTDAL_ASSERT_PARAM_P(id);
	int i;
	for (i=0;i<MAX(rtdal_itfphysic);i++) {
		if (context->physic_itfs[i].parent.id == id)
			break;
	}

	hdebug("i=%d\n",i);
	if (i == MAX(rtdal_itfphysic)) {
		RTDAL_SETERROR(RTDAL_ERROR_NOTFOUND);
		return NULL;
	}

	return (r_itf_t) &context->physic_itfs[i];
}


/**
 * Returns a handler to the first dac object matching the name.
 * @param name Name of the dac
 * @return non-null value on success, zero on error
 */
r_dac_t rtdal_dac_get(string address) {
	assert(context);
	aerror("Not yet implemented");
	return NULL;
}


/**
 * Creates a new process to be executed by one of the kernel pipelines.
 * The structure pointed by attr is defined in rtdal_process_attr.

 *
 * rtdal_process_new() configures the process parameters and loads it into the
 * pipeline. The process won't execute until a success call to rtdal_process_run()
 * is performed.
 *
 * The address pointed by arg will be passed to the process as a parameter.
 *
 * Notes: The shared library must conform with the rtdal_process_t requirements.
 *
 * @param attr Configures the process attributes
 * @param arg Argument to pass to the process each execution cycle
 * @return non-null value on success, zero on error
 */
r_proc_t rtdal_process_new(struct rtdal_process_attr *attr, void *arg) {
	hdebug("binary=%s, proc=%d, pos=%d, finish=0x%x, arg=0x%x\n",attr->binary_path,attr->pipeline_id,
			attr->exec_position,attr->finish_callback,arg);
	assert(context);
	RTDAL_ASSERT_PARAM_P(attr);
	RTDAL_ASSERT_PARAM_P(arg);

	pthread_mutex_lock(&context->mutex);
	int i=0;
	/* find empty space on process db */
	for (i=0;i<MAX(rtdal_process);i++) {
		if (!context->processes[i].pid)
			break;
	}

	if (i == MAX(rtdal_process)) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		goto out;
	}

	if (attr->process_group_id > MAX_PROCESS_GROUP_ID) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		goto out;
	}

	if (attr->process_group_id < 0) {
		RTDAL_SETERROR(RTDAL_ERROR_INVAL);
		goto out;
	}

	pgroup_notified_failure[attr->process_group_id] = 0;
	memset(&context->processes[i],0,sizeof(rtdal_process_t));
	context->processes[i].pid=i+1;

	hdebug("i=%d, pid=%d\n",context->processes[i].pid);

	memcpy(&context->processes[i].attributes, attr,
			sizeof(struct rtdal_process_attr));
	context->processes[i].arg = arg;
	context->processes[i].finish_code = FINISH_OK;

	switch(context->machine.scheduling) {
		case SCHEDULING_PIPELINE:
			if (pipeline_add(&context->pipelines[attr->pipeline_id],
					&context->processes[i]) == -1) {
				goto out;
			}
		break;
		case SCHEDULING_BESTEFFORT:
			if (modulethread_new(&context->processes[i])) {
				goto out;
			}
			break;
	}

	if (rtdal_process_launch(&context->processes[i])) {
		rtdal_process_remove((r_proc_t)&context->processes[i]);
		goto out;
	}


	pthread_mutex_unlock(&context->mutex);
	return (r_proc_t) &context->processes[i];
out:
	pthread_mutex_unlock(&context->mutex);
	return NULL;
}


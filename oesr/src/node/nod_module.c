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
#include <string.h>
#include <unistd.h>

#include "rtdal.h"
#include "defs.h"
#include "packet.h"
#include "nod_waveform.h"
#include "mempool.h"
#include "oesr_context.h"
#include "nod_anode.h"

extern r_log_t modules_log;
extern struct log_cfg logs_cfg;

/**  Allocates memory for the oesr context structure of a nod_module_t
 *
 */
int nod_module_alloc(nod_module_t *module) {
	ndebug("module_addr=0x%x\n",module);
	aassert(module);

	module->context = pool_alloc(1,oesr_sizeof());
	if (!module->context) {
		return -1;
	}

	if (oesr_context_init(module->context, module)) {
		return -1;
	}

	memset(&module->parent.execinfo,0,sizeof(execinfo_t));
	return 0;
}

/**  Deallocates the oesr structure memory allocated using nod_module_alloc()
 *
 */
int nod_module_free(nod_module_t *module) {
	ndebug("module_id=%d, addr=0x%x, context=0x%x\n",module->parent.id,module,module->context);
	aassert(module);

	if (module->context) {
		pool_free(module->context);
	}
	module->context = NULL;
	return 0;
}


/**
 *  This function is called by the rtdal when a module finishes the execution. It is always
 * called from a non-priority task, therefore there are no time constraints.
 *
 * If process.finishCode!=ProcessErrorCode.OK, an excepcional error occurred.
 * The reason is stored in process.errorMsg string.
 */
void *nod_module_finish_callback(void *context) {
	nod_module_t *module = oesr_get_module(context);
	nod_waveform_t *waveform = module->parent.waveform;
	ndebug("module_id=%d, error_code=%d, finishing=%d, runnable=%d\n",module->parent.id,
			rtdal_process_geterror(module->process), waveform->finishing);

	switch(rtdal_process_geterror(module->process)) {

	case SIG_RECV:
		aerror_msg("Module %s produced a SIGSEGV, SIGILL, SIGFPE or SIGBUS signal. Trying a clean stop\n",
						module->parent.name);
		break;
	case RTFAULT:
	case RUNERROR:
		ndebug("Module %s returned error from work() function. Trying a clean stop\n",
						module->parent.name);
		rtdal_printf("#");
		break;
	default:
			aerror("Warning, not supposed to be here\n");
			return 0;
		break;
	}

	if (rtdal_process_geterror(module->process) == RUNERROR
			|| rtdal_process_geterror(module->process) == RTFAULT) {
#ifdef FINISH_ON_RUNERROR
		if (nod_waveform_status_stop(waveform)) {
			aerror("stopping waveform\n");
		}
#else
		if (nod_waveform_reset_pipeline(waveform,module->process)) {
			aerror("resetting pipeline\n");
		}
#endif

	} else {
		if (nod_waveform_status_stop(waveform)) {
			aerror("stopping waveform\n");
		}
	}
	return NULL;
}

/**  nod_module_load() initializes the module's oesr context and then uses the rtdal
 * to create a new process that will run the module functions. The oesr context pointer is passed
 * as a parameter to the rtdal_process_new(). This pointer will be passed to the module entry point
 * functions.
 * \returns 0 on success, -1 on error
 */
int nod_module_load(nod_module_t *module) {
	ndebug("module_id=%d, binary=%s, exe_pos=%d, pidx=%d, context=0x%x\n",module->parent.id,
			module->parent.binary, module->parent.exec_position,module->parent.processor_idx,
			module->context);
	aassert(module);
	struct rtdal_process_attr attr;

	memset(&attr, 0, sizeof(struct rtdal_process_attr));
	strcpy(attr.binary_path,module->parent.binary);
	attr.exec_position = module->parent.exec_position;
	attr.pipeline_id = module->parent.processor_idx;
	attr.finish_callback = nod_module_finish_callback;

	nod_waveform_t *waveform = module->parent.waveform;
	attr.process_group_id = waveform->id;

	char tmp[128];
	if (logs_cfg.modules_en && (module->parent.log_enable & 0x1 || logs_cfg.modules_all)) {
		if (modules_log) {
			module->log = modules_log;
		} else {
			snprintf(tmp,128,"%s.log",module->parent.name);
			module->log = rtdal_log_new(tmp,TEXT,0);
			if (!module->log) {
				aerror_msg("Creating module log %s\n",tmp);
			}
		}
		module->log_level = logs_cfg.log_modules_level;
		module->output_stdout = logs_cfg.log_to_stdout;
	} else {
		module->log = NULL;
	}
	if (logs_cfg.modules_time_en && (module->parent.log_enable & 0x2 || logs_cfg.modules_all)) {
		snprintf(tmp,128,"%s.time",module->parent.name);
		module->time_log = rtdal_log_new(tmp,INT32,0);
		if (!module->time_log) {
			aerror_msg("Creating module log %s\n",tmp);
		}
	} else {
		module->time_log = NULL;
	}

	module->init = NULL;
	module->stop = NULL;

	module->process = rtdal_process_new(&attr, module->context);
	if (module->process == NULL) {
		aerror_msg("Error loading module %s\n",module->parent.name);
		rtdal_perror("rtdal_process_new");
		return -1;
	}
	return 0;
}

/**  Sets the module's process as runnable
 *
 */
int nod_module_run(nod_module_t *module, int runnable) {
	ndebug("module_id=%d\n",module->parent.id);
	aassert(module);
	if (runnable) {
		if (rtdal_process_run(module->process)) {
			rtdal_perror("rtdal_process_run");
			return -1;
		}
	} else {
		if (rtdal_process_stop(module->process)) {
			rtdal_perror("rtdal_process_run");
			return -1;
		}
	}
	return 0;
}


/**  Removes the module's from the rtdal pipeline. Calls module_free() to dealloc
 * the interfaces/variables memory and then nod_module_free() to dealloc the oesr context memory.
 * Sets the id to zero before finishing.
 */
int nod_module_remove(nod_module_t *module) {
	ndebug("module_id=%d\n",module->parent.id);
	aassert(module);

	if (module->process) {
		if (rtdal_process_remove(module->process)) {
			rtdal_perror("rtdal_process_remove");
			return -1;
		}
	}
	if (module_free(&module->parent)) {
		return -1;
	}
	if (nod_module_free(module)) {
		return -1;
	}
	module->changing_status = 0;
	module->parent.id = 0;
	return 0;
}


int nod_module_init(nod_module_t *module) {
	ndebug("module_id=%d status=%d\n",module->parent.id,module->parent.status);
	if (!module->init) {
		aerror_msg("Init function unregistered for module_id=%d\n",module->parent.id);
		return -1;
	}
	return module->init(module);
}

int nod_module_stop(nod_module_t *module) {
	ndebug("module_id=%d status=%d\n",module->parent.id,module->parent.status);
	if (!module->stop) {
		aerror_msg("Stop function unregistered for module_id=%d\n",module->parent.id);
		return -1;
	}
	if (module->stop(module)) {
		aerror_msg("stopping module_id=%d\n",module->parent.id);
		return -1;
	}
	return 0;
}


/** Returns a pointer to the first module's variable with name equal to the second parameter
 *
 */
variable_t* nod_module_variable_get(nod_module_t *module, string name) {
	ndebug("module_id=%d, nof_variables=%d, name=%s\n",module->parent.id,
			module->parent.nof_variables, name);
	aassert_p(module);
	aassert_p(name);
	int i;
	i=0;
	while(i < module->parent.nof_variables && strcmp(name,module->parent.variables[i].name)) {
		i++;
		ndebug("compare %s==%s\n", name,module->parent.variables[i].name);
	}
	if (i == module->parent.nof_variables) {
		return NULL;
	}
	return &module->parent.variables[i];
}

/** Returns a pointer to the first empty variable in the module structure. Fills
 * the variable name with the second parameter string and sets the variable id to a non-zero integer.
 */
variable_t* nod_module_variable_create(nod_module_t *module, string name, int size) {
	ndebug("module_id=%d, nof_variables=%d, name=%s\n",module->parent.id,
				module->parent.nof_variables, name);
	int i;
	if (!module || !name) {
		return NULL;
	}
	i=0;
	while(i < module->parent.nof_variables && module->parent.variables[i].id)
		i++;
	if (i == module->parent.nof_variables) {
		aerror("Can't create more variables. Increase NOF_USER_VARIABLES in oesr/common/waveform.h\n");
		return NULL;
	}

	if (size) {
		module->parent.variables[i].size = size;
		variable_alloc(&module->parent.variables[i], module->parent.nof_modes);
	}

	module->parent.variables[i].id = i+1;
	strncat(module->parent.variables[i].name,name,STR_LEN-1-strnlen(name,STR_LEN));

	return &module->parent.variables[i];
}


int nod_module_execinfo_add_sample(execinfo_t *obj, int ctx_tstamp) {
	int tstamp = rtdal_time_slot();
	int cpu = obj->t_exec[0].tv_usec;
	int relinquish = obj->t_exec[2].tv_usec;
	int start = obj->t_exec[1].tv_usec;

#ifdef RELINQUISH_DO_MOD
	rtdal_machine_t machine;
	rtdal_machine(&machine);
	relinquish = relinquish % machine.ts_len_us;
	start = start % machine.ts_len_us;
#endif

	if (!obj->start_ts) {
		obj->start_ts = tstamp;
	}
	if (cpu > obj->max_exec_us) {
		obj->max_exec_us = cpu;
		obj->max_exec_ts = ctx_tstamp;
	}
	obj->module_ts = ctx_tstamp;
	obj->max_rel_us = relinquish > obj->max_rel_us ? relinquish : obj->max_rel_us;
	obj->max_start_us = start > obj->max_start_us ? start : obj->max_start_us;

	if (tstamp - obj->start_ts + 1 > 0) {
		obj->mean_exec_us = (float) obj->mean_exec_us +
				(float) (cpu - obj->mean_exec_us)/(tstamp - obj->start_ts + 1);
		obj->mean_rel_us = (float) obj->mean_rel_us +
				(float) (relinquish - obj->mean_rel_us)/(tstamp - obj->start_ts + 1);
		obj->mean_start_us = (float) obj->mean_start_us +
				(float) (start - obj->mean_start_us)/(tstamp - obj->start_ts + 1);
	}
	return 0;
}



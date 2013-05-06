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

#include <stdlib.h>
#include <libconfig.h>

#include <rtdal.h>

#include "defs.h"
#include "objects_max.h"
#include "oesr.h"
#include "nod_anode.h"
#include "nod_dispatcher.h"
#include "mempool.h"
#include "packet.h"

packet_t *node_packet;

nod_anode_t anode;

r_log_t oesr_log, modules_log, queues_log;

struct log_cfg logs_cfg;

void nod_anode_initialize_waveforms(int max_waveforms) {
	ndebug("max_waveforms=%d\n",max_waveforms);

	int i;

	anode.loaded_waveforms = (nod_waveform_t*) pool_alloc(max_waveforms,sizeof(waveform_t));
	assert(anode.loaded_waveforms);
	anode.max_waveforms = max_waveforms;

	for (i=0;i<max_waveforms;i++) {
		memset(&anode.loaded_waveforms[i],0,sizeof(nod_waveform_t));
		anode.loaded_waveforms[i].status.cur_status = STOP;
	}
}

int nod_anode_parse_cfg(char *config_file) {
	config_t config;
	int ret = -1;
	const char *tmp;
	config_setting_t *cfg;

	config_init(&config);
	if (!config_read_file(&config, config_file)) {
		aerror_msg("line %d - %s: \n", config_error_line(&config),
				config_error_text(&config));
		goto destroy;
	}
	cfg = config_lookup(&config, "other");
	if (!cfg) {
		goto destroy;
	}
	if (!config_setting_lookup_bool(cfg,"log_oesr_en",&logs_cfg.enabled)) {
		logs_cfg.enabled=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_modules_to_stdout",&logs_cfg.log_to_stdout)) {
		logs_cfg.log_to_stdout=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_modules",&logs_cfg.modules_en)) {
		logs_cfg.modules_en=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_modules_all",&logs_cfg.modules_all)) {
		logs_cfg.modules_all=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_modules_join",&logs_cfg.modules_join)) {
		logs_cfg.modules_join=0;
	}
	if (!config_setting_lookup_string(cfg,"log_modules_level",&tmp)) {
		logs_cfg.log_modules_level=1;
	}
	if (!strcmp(tmp,"info")) {
		logs_cfg.log_modules_level=LOG_LEVEL_INFO;
	} else if (!strcmp(tmp,"itf")) {
		logs_cfg.log_modules_level=LOG_LEVEL_ITF;
	} else if (!strcmp(tmp,"itf-info")) {
		logs_cfg.log_modules_level=LOG_LEVEL_INFO|LOG_LEVEL_ITF;
	} else if (!strcmp(tmp,"debug")) {
		logs_cfg.log_modules_level=LOG_LEVEL_INFO|LOG_LEVEL_ITF|LOG_LEVEL_DEBUG;
	}
	if (!config_setting_lookup_bool(cfg,"log_queues",&logs_cfg.queues_en)) {
		logs_cfg.queues_en=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_queues_all",&logs_cfg.queues_all)) {
		logs_cfg.queues_all=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_queues_join",&logs_cfg.queues_join)) {
		logs_cfg.queues_join=0;
	}
	if (!config_setting_lookup_bool(cfg,"trace_modules_exetime",&logs_cfg.modules_time_en)) {
		logs_cfg.modules_time_en=0;
	}
	if (!config_setting_lookup_bool(cfg,"join_logs_sync",&logs_cfg.join_logs_sync)) {
		logs_cfg.join_logs_sync=0;
	}

	ret=0;
destroy:
	config_destroy(&config);
	return ret;
}

/**
 * 1) Pre-allocates max_waveforms objects of type Waveform
 * (with max_modules_x_waveform and max_variables_x_module modules and variables object instances)
 * in the +loaded_waveforms array and initialize synchronization master/slave
 * 2) Find physical interface with name "ctrl" and save to ctrlItf
 * 3) Find physical interface with name "probe" and save to probeItf
 * 4) If !rtdal.machine.syncMaster,
 *    4.1) find physical interface with name "sync" and save pointer to syncItf                              
 *    4.2) syncItf.setCallback(SyncSlave,kernelPrio)
 *    4.3) if !rtdal.machine.syncContinuous add periodic function SyncRequest() to the kernel
 * with period "period"
 * 5) If rtdal.machine.syncMaster,
 *    5.1) find all physical interface with name starting with "slavesync*" and save their object
 *  address to slaveItf array
 *    5.2) if rtdal.machine.isContinuous add SyncMaster() as a periodic function to the kernel with
 * period 1
 *    5.3) if !rtdal.machine.isContinuous slaveItf[i].setCallback(SyncMaster,kernelPrio) for each
 * slaveItf
 */
int nod_anode_initialize(rtdal_machine_t *machine, int max_waveforms) {
	ndebug("max_waveforms=%d\n",max_waveforms);
	int opts;

	node_packet = &anode.packet;
	if (packet_init(&anode.packet, 512*1024)) {
		aerror("initializing packet\n");
		return -1;
	}

	if (nod_anode_parse_cfg(machine->cfg_file)) {
		aerror_msg("Missing other section in configuration file %s\n",machine->cfg_file);
	}

	if (logs_cfg.join_logs_sync) {
		opts = RTDAL_LOG_OPTS_EXCL;
	} else {
		opts = 0;
	}

	if (logs_cfg.enabled) {
		oesr_log = rtdal_log_new("oesr.log",TEXT,0);
		if (!oesr_log) {
			aerror("Initializing oesr log\n");
			return -1;
		}
	} else {
		oesr_log = NULL;
	}
	if (logs_cfg.modules_en && logs_cfg.modules_join) {
		modules_log = rtdal_log_new_opts("modules.log",TEXT,0,opts);
		if (!modules_log) {
			aerror("Initializing modules log\n");
			return -1;
		}
	} else {
		modules_log = NULL;
	}
	if (logs_cfg.queues_en && logs_cfg.queues_join) {
		queues_log = rtdal_log_new_opts("queues.log",TEXT,0,opts);
		if (!queues_log) {
			aerror("Initializing queues log\n");
			return -1;
		}
	} else {
		queues_log = NULL;
	}

	nod_anode_initialize_waveforms(max_waveforms);
	return 0;
}


/**
 * This is a thread created by the rtdal kernel thread with normal priority. Reads commands from
 * ctrl interface and processes them.
 */
int nod_anode_cmd_recv() {
	/**@TODO: This should be a thread. Send nod_anode_dispatch() return value using ACK */
	return nod_anode_dispatch(&anode.packet);
}


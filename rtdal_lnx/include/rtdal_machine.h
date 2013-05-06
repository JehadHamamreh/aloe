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

#ifndef rtdal_MACHINE_H
#define rtdal_MACHINE_H


#ifdef HAVE_UHD
#include "dac_cfg.h"
#include "uhd.h"
#endif


#include "str.h"

enum clock_source {
	SINGLE_TIMER, SYNC_SLAVE, MULTI_TIMER, DAC
};

#define RT_FAULT_OPTS_HARD 1
#define RT_FAULT_OPTS_SOFT 2

#define RTDAL_MAX_CORES		16

struct rtdal_logs_cfg {
	int enabled;
	int kernel_en;
	int timing_en;
	int log_to_stout;
	int log_length_mb;
	lstrdef(base_path);
};

struct rtfault_cfg {
	int exec_correct;
	int miss_correct;
	int exec_kill;
	int miss_kill;
	int do_rtcontrol;
};

/**
 * Public structure configured at initialize() from the information read from platform.conf. Stores some properties of the local machine architecture.
 */
typedef struct {
	long int ts_len_us;
	strdef(cfg_file);
	int cpu_type;
	float mopts;
	float mbpts;
	string name;
	int core_mapping[RTDAL_MAX_CORES];
	int nof_cores;
	int using_uhd;
	int rt_fault_opts;
	int kernel_prio;
	int pipeline_prio;
	int sync_period;
	int sync_continuous;
	int slave_master;
	int max_waveforms;
	int max_modules_x_waveform;
	int max_variables_x_module;
	struct rtdal_logs_cfg logs_cfg;
	enum clock_source clock_source;
	struct rtfault_cfg rt_cfg;
	lstrdef(path_to_libs);
	void (*slave_sync_kernel) (void*, struct timespec *time);
}rtdal_machine_t;

#endif

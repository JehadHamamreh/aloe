#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <libconfig.h>
#include <assert.h>

#include "rtdal_kernel.h"
#include "rtdal_machine.h"
#include "defs.h"

int parse_cores_comma_sep(char *str, int *core_mapping) {
	int i;
	char *tok;

	/* assume 10 cores */
	i=0;
	tok = strtok(str,",");
	while (tok) {
		core_mapping[i] = atoi(tok);
		tok = strtok(0,",");
		i++;
		if ((size_t) i > RTDAL_MAX_CORES) {
			aerror_msg("Can't parse more cores. "
					"Increase RTDAL_MAX_CORES=%d in rtdal_lnx/include/rtdal_machine.h",
					RTDAL_MAX_CORES);
			return -1;
		}
	}

	return i;
}

int parse_cores_single_array(char *core_init, char *core_end, int *core_mapping) {
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
	if (c_end-c_ini > RTDAL_MAX_CORES) {
		aerror_msg("Can't parse more cores. "
				"Increase RTDAL_MAX_CORES=%d in rtdal_lnx/include/rtdal_machine.h",
				RTDAL_MAX_CORES);
		return -1;
	}
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
int parse_cores(char *str, int *core_mapping) {
	char *dp;
	char *c;

	dp = index(str,':');
	c = index(str,',');

	if (!c && !dp) {
		return parse_cores_single_array(NULL,str,core_mapping);
	} else if (!c && dp) {
		*dp = '\0';
		dp++;
		return parse_cores_single_array(str,dp,core_mapping);
	} else if (c && !dp) {
		return parse_cores_comma_sep(str,core_mapping);
	} else {
		return -1;
	}
}


int parse_config_opts(config_setting_t *cfg, rtdal_machine_t *machine) {
	const char *tmp;

	if (!config_setting_lookup_bool(cfg,"log_enabled",&machine->logs_cfg.enabled)) {
		machine->logs_cfg.enabled=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_to_stout",&machine->logs_cfg.log_to_stout)) {
		machine->logs_cfg.log_to_stout=0;
	}

	if (!config_setting_lookup_string(cfg, "log_directory", &tmp)) {
		strcpy(machine->logs_cfg.base_path,"reports");
	} else {
		strcpy(machine->logs_cfg.base_path,tmp);
	}
	if (!config_setting_lookup_bool(cfg,"log_rtdal_en",&machine->logs_cfg.kernel_en)) {
		machine->logs_cfg.kernel_en=0;
	}
	if (!config_setting_lookup_bool(cfg,"log_timing_en",&machine->logs_cfg.timing_en)) {
		machine->logs_cfg.timing_en=0;
	}
	if (!config_setting_lookup_int(cfg,"log_length_mb",&machine->logs_cfg.log_length_mb)) {
		machine->logs_cfg.log_length_mb=0;
	}

	if (!machine->logs_cfg.enabled) {
		memset(&machine->logs_cfg,0,sizeof(struct rtdal_logs_cfg));
	}

	if (!config_setting_lookup_bool(cfg,"xenomai_warn_msw",&machine->rt_cfg.xenomai_warn_msw)) {
		machine->rt_cfg.xenomai_warn_msw=0;
	}

	return 0;
}

int parse_pipeline_opts(config_setting_t *cfg, rtdal_machine_t *machine) {
	const char *tmp;
	int time_slot_ns_int;

	if (!config_setting_lookup_int(cfg, "time_slot_ns", &time_slot_ns_int)) {
		aerror("time_slot_us field not defined\n");
		return -1;
	}
	machine->ts_len_ns = (long int) time_slot_ns_int;

	if (!config_setting_lookup_string(cfg, "cores", &tmp)) {
		aerror("cores field not defined\n");
		return -1;
	}
	machine->nof_cores = parse_cores((char*) tmp, machine->core_mapping);
	if (machine->nof_cores < 0) {
		printf("Error invalid cores %s\n",tmp);
		return -1;
	}

	if (!config_setting_lookup_string(cfg, "timer_mode", &tmp)) {
		aerror("timer_mode_single field not defined\n");
		return -1;
	}
	if (!strcmp(tmp,"single")) {
		machine->clock_mode = SINGLE_TIMER;
	} else if (!strcmp(tmp,"multi")) {
		machine->clock_mode = MULTI_TIMER;
	} else if (!strcmp(tmp,"none")) {
		machine->clock_mode = NO_TIMER;
	} else {
		aerror_msg("Invalid timer mode %s\n",tmp);
		return -1;
	}
	double t;
	if (!config_setting_lookup_float(cfg,"core0_relative",&t)) {
		machine->core0_relative=1.0;
	} else {
		machine->core0_relative=(float) t;
	}

	if (!config_setting_lookup_bool(cfg,"thread_sync_on_finish",&machine->thread_sync_on_finish)) {
		machine->thread_sync_on_finish=0;
	}

	if (!config_setting_lookup_bool(cfg,"correct_on_rtfault_missed",&machine->rt_cfg.miss_correct)) {
		machine->rt_cfg.miss_correct=0;
	}
	if (!config_setting_lookup_bool(cfg,"correct_on_rtfault_exec",&machine->rt_cfg.exec_correct)) {
		machine->rt_cfg.exec_correct=0;
	}
	if (!config_setting_lookup_bool(cfg,"kill_on_rtfault_missed",&machine->rt_cfg.miss_kill)) {
		machine->rt_cfg.miss_kill=0;
	}
	if (!config_setting_lookup_bool(cfg,"kill_on_rtfault_exec",&machine->rt_cfg.exec_kill)) {
		machine->rt_cfg.exec_kill=0;
	}
	return 0;
}

int parse_config(char *config_file, rtdal_machine_t *machine) {
	config_t config;
	int ret = -1;
	config_setting_t *rtdal,*rtdal_opts,*pipeline_opts;
	const char *tmp;

	config_init(&config);
	if (!config_read_file(&config, config_file)) {
		aerror_msg("line %d - %s: \n", config_error_line(&config),
				config_error_text(&config));
		goto destroy;
	}
	strcpy(machine->cfg_file,config_file);

	rtdal_opts = config_lookup(&config, "rtdal_opts");
	if (rtdal_opts) {
		parse_config_opts(rtdal_opts,machine);
	}

	rtdal = config_lookup(&config, "rtdal");
	if (!rtdal) {
		aerror("Error parsing config file: rtdal section not found.\n");
		goto destroy;
	}

	if (!config_setting_lookup_string(rtdal, "path_to_libs", &tmp)) {
		aerror("path_to_libs field not defined\n");
		goto destroy;
	}

	strcpy(machine->path_to_libs,tmp);

	if (!config_setting_lookup_string(rtdal, "scheduling", &tmp)) {
		aerror("cores field not defined\n");
		goto destroy;
	}

	if (!strcmp(tmp,"pipeline")) {
		pipeline_opts = config_lookup(&config, "pipeline_opts");
		if (!pipeline_opts) {
			aerror("Error parsing config file: pipeline_opts section not found but "
					"pipeline scheduling was selected.\n");
			goto destroy;
		}
		machine->scheduling = SCHEDULING_PIPELINE;
		machine->queues = QUEUE_NONBLOCKING;
		if (parse_pipeline_opts(pipeline_opts, machine)) {
			goto destroy;
		}
	} else if (!strcmp(tmp,"best-effort")) {
		machine->scheduling = SCHEDULING_BESTEFFORT;
		machine->queues = QUEUE_BLOCKING;
	} else {
		aerror_msg("Invalid scheduling %s\n",tmp);
		goto destroy;
	}

	if (machine->rt_cfg.miss_correct || machine->rt_cfg.exec_correct
			|| machine->rt_cfg.miss_kill || machine->rt_cfg.exec_kill
			|| (machine->logs_cfg.timing_en && machine->logs_cfg.enabled)) {
		machine->rt_cfg.do_rtcontrol = 1;
	}

	/* Initialize rtdal_base library */
	machine->kernel_prio = KERNEL_RT_PRIO;
	machine->rt_fault_opts = RT_FAULT_OPTS_HARD;

	ret=0;
destroy:
	config_destroy(&config);
	return ret;
}

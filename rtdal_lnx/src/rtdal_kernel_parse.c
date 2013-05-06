#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <libconfig.h>
#include <assert.h>

#include "rtdal_kernel.h"
#include "rtdal_machine.h"
#include "defs.h"


#ifdef HAVE_UHD
	extern struct dac_cfg dac_cfg;
#endif

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
	if (machine->rt_cfg.miss_correct || machine->rt_cfg.exec_correct
			|| machine->rt_cfg.miss_kill || machine->rt_cfg.exec_kill
			|| (machine->logs_cfg.timing_en && machine->logs_cfg.enabled)) {
		machine->rt_cfg.do_rtcontrol = 1;
	}
	return 0;
}

int parse_config(char *config_file, rtdal_machine_t *machine) {
	config_t config;
	int ret = -1;
	config_setting_t *rtdal,*rtdal_opts,*dac;
	const char *tmp;
	int single_timer,time_slot_us_int;

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

	if (!config_setting_lookup_int(rtdal, "time_slot_us", &time_slot_us_int)) {
		aerror("time_slot_us field not defined\n");
		goto destroy;
	}
	machine->ts_len_us = (long int) time_slot_us_int;

	if (!config_setting_lookup_string(rtdal, "cores", &tmp)) {
		aerror("cores field not defined\n");
		goto destroy;
	}
	machine->nof_cores = parse_cores((char*) tmp, machine->core_mapping);
	if (machine->nof_cores < 0) {
		printf("Error invalid cores %s\n",tmp);
		exit(0);
	}

	if (!config_setting_lookup_bool(rtdal, "enable_usrp", &machine->using_uhd)) {
		aerror("enable_usrp field not defined\n");
		goto destroy;
	}

	if (!config_setting_lookup_bool(rtdal, "timer_mode_single", &single_timer)) {
		aerror("timer_mode_single field not defined\n");
		goto destroy;
	}
	if (machine->using_uhd) {
		if (single_timer) {
			machine->clock_source = SINGLE_TIMER;
		} else {
			machine->clock_source = DAC;
		}
	} else {
		if (single_timer) {
			machine->clock_source = SINGLE_TIMER;
		} else {
			machine->clock_source = MULTI_TIMER;
		}
	}
	if (!config_setting_lookup_string(rtdal, "path_to_libs", &tmp)) {
		aerror("path_to_libs field not defined\n");
		goto destroy;
	}

	strcpy(machine->path_to_libs,tmp);

	if (machine->using_uhd) {
		dac = config_lookup(&config, "dac");
		if (!dac) {
			aerror("Error parsing config file: dac section not found.\n");
			goto destroy;
		}

#ifdef HAVE_UHD
		double tmp;
		if (!config_setting_lookup_float(dac, "samp_freq", &dac_cfg.inputFreq)) {
			aerror("samp_freq field not defined\n");
			goto destroy;
		}
		dac_cfg.outputFreq = dac_cfg.inputFreq;

		if (!config_setting_lookup_float(dac, "rf_freq", &dac_cfg.inputRFFreq)) {
			aerror("rf_freq field not defined\n");
			goto destroy;
		}
		dac_cfg.outputRFFreq = dac_cfg.inputRFFreq;

		if (!config_setting_lookup_float(dac, "rf_gain", &tmp)) {
			aerror("rf_gain field not defined\n");
			goto destroy;
		}
		dac_cfg.tx_gain = tmp;
		dac_cfg.rx_gain = tmp;

		if (!config_setting_lookup_float(dac, "if_bw", &tmp)) {
			aerror("rf_gain field not defined\n");
			goto destroy;
		}
		dac_cfg.tx_bw = tmp;
		dac_cfg.rx_bw = tmp;

		if (!config_setting_lookup_bool(dac, "sample_is_short", &dac_cfg.sampleType)) {
			aerror("rf_gain field not defined\n");
			goto destroy;
		}

		if (!config_setting_lookup_int(dac, "block_size", &dac_cfg.NsamplesIn)) {
			aerror("block_size field not defined\n");
			goto destroy;
		}
		dac_cfg.NsamplesOut = dac_cfg.NsamplesIn;

		if (!config_setting_lookup_bool(dac, "chain_is_tx", &dac_cfg.chain_is_tx)) {
			aerror("chain_is_tx field not defined\n");
			goto destroy;
		}

		dac_cfg.sampleType = 0;
		dac_cfg.nof_channels = 1;

		uhd_readcfg(&dac_cfg);
#endif
	}
	if (machine->using_uhd) {
#ifdef HAVE_UHD
		machine->ts_len_us = (long int) 1000000*((float) dac_cfg.NsamplesOut/dac_cfg.outputFreq);
#endif
	}

	/* Initialize rtdal_base library */
	machine->kernel_prio = KERNEL_RT_PRIO;
	machine->rt_fault_opts = RT_FAULT_OPTS_HARD;

	ret=0;
destroy:
	config_destroy(&config);
	return ret;
}

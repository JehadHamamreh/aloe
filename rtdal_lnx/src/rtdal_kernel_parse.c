#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <libconfig.h>
#include <assert.h>

#include "rtdal_kernel.h"
#include "defs.h"

#ifdef HAVE_UHD
#include "dac_cfg.h"
#include "uhd.h"
extern struct dac_cfg dac_cfg;
#endif

char libs_path[255];

extern int *core_mapping;
extern int nof_cores;

extern long int timeslot_us;
extern enum clock_source clock_source;

extern int using_uhd;

int parse_cores_comma_sep(char *str) {
	int i;
	size_t sz;
	char *tok;

	/* assume 10 cores */
	sz = 10;
	assert((core_mapping = malloc(sizeof(int)*sz)));
	i=0;
	tok = strtok(str,",");
	while (tok) {
		core_mapping[i] = atoi(tok);
		tok = strtok(0,",");
		i++;
		if ((size_t) i > sz) {
			sz += 10;
			assert((core_mapping = realloc(core_mapping, sizeof(int)*sz)));
		}
	}
	assert((core_mapping = realloc(core_mapping, sizeof(int)*(size_t)i)));

	return i;
}

int parse_cores_single_array(char *core_init, char *core_end) {
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
	core_mapping = malloc(sizeof(int)*((size_t) c_end-(size_t)c_ini));
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
int parse_cores(char *str) {
	char *dp;
	char *c;

	dp = index(str,':');
	c = index(str,',');

	if (!c && !dp) {
		return parse_cores_single_array(NULL,str);
	} else if (!c && dp) {
		*dp = '\0';
		dp++;
		return parse_cores_single_array(str,dp);
	} else if (c && !dp) {
		return parse_cores_comma_sep(str);
	} else {
		return -1;
	}
}

int parse_config(char *config_file) {
	config_t config;
	int ret = -1;
	config_setting_t *rtdal,*dac;
	const char *tmp;
	int single_timer;
	int time_slot_us;

	config_init(&config);
	if (!config_read_file(&config, config_file)) {
		aerror_msg("line %d - %s: \n", config_error_line(&config),
				config_error_text(&config));
		goto destroy;
	}

	rtdal = config_lookup(&config, "rtdal");
	if (!rtdal) {
		aerror("Error parsing config file: rtdal section not found.\n");
		goto destroy;
	}

	if (!config_setting_lookup_int(rtdal, "time_slot_us", &time_slot_us)) {
		aerror("time_slot_us field not defined\n");
		goto destroy;
	}

	if (!config_setting_lookup_string(rtdal, "cores", &tmp)) {
		aerror("cores field not defined\n");
		goto destroy;
	}
	nof_cores = parse_cores((char*) tmp);
	if (nof_cores < 0) {
		printf("Error invalid cores %s\n",tmp);
		exit(0);
	}

	if (!config_setting_lookup_bool(rtdal, "enable_usrp", &using_uhd)) {
		aerror("enable_usrp field not defined\n");
		goto destroy;
	}

	if (!config_setting_lookup_bool(rtdal, "timer_mode_single", &single_timer)) {
		aerror("timer_mode_single field not defined\n");
		goto destroy;
	}
	if (using_uhd) {
		if (single_timer) {
			clock_source = SINGLE_TIMER;
		} else {
			clock_source = DAC;
		}
	} else {
		if (single_timer) {
			clock_source = SINGLE_TIMER;
		} else {
			clock_source = MULTI_TIMER;
		}
	}
	if (!config_setting_lookup_string(rtdal, "path_to_libs", &tmp)) {
		aerror("path_to_libs field not defined\n");
		goto destroy;
	}

	strcpy(libs_path,tmp);

	if (using_uhd) {
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
	if (using_uhd) {
#ifdef HAVE_UHD
		timeslot_us = (long int) 1000000*((float) dac_cfg.NsamplesOut/dac_cfg.outputFreq);
#endif
	} else {
		timeslot_us = time_slot_us;
	}
	ret=0;
destroy:
	config_destroy(&config);
	return ret;
}

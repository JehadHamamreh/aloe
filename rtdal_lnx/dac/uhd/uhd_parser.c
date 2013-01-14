#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dac_cfg.h"
#include "uhd.h"

struct main_conf main_cfg;

char filepath[200];

int uhd_readcfg(struct dac_cfg *dac_cfg) {

	dac_cfg->inputFreq = 1923077;
	dac_cfg->outputFreq = 1923077;
	dac_cfg->inputRFFreq = 1000000000;
	dac_cfg->outputRFFreq = 1000000000;
	dac_cfg->tx_gain = 0.0;
	dac_cfg->rx_gain = 0.0;
	dac_cfg->sampleType = 0;
	dac_cfg->nof_channels = 1;
	dac_cfg->NsamplesIn = 960;
	dac_cfg->NsamplesOut = 960;

	strcat(main_cfg.clock,"internal");
	strcat(main_cfg.clock,"not_used");
	main_cfg.chain_is_tx = 1;

	uhd_setcfg(&main_cfg,dac_cfg);
	return 1;

}

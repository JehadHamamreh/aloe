#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dac_cfg.h"
#include "uhd.h"

struct main_conf main_cfg;

char filepath[200];

int uhd_readcfg(struct dac_cfg *dac_cfg) {

	strcat(main_cfg.clock,"internal");
	main_cfg.chain_is_tx = dac_cfg->chain_is_tx;

	uhd_setcfg(&main_cfg,dac_cfg);
	return 1;

}

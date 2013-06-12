#include <stdio.h>
#include <string.h>
#include "base.h"


/** 36.211 10.3 section 6.7.4
 */
int lte_grid_init_reg_pcfich(struct lte_grid_config *config) {
	int i,kh,k,modulo;

	modulo = config->nof_prb*NOF_RE_X_OSYMB;
	kh=(NOF_RE_X_OSYMB/2)*(config->cell_id%(2*config->nof_prb));
	for (i=0;i<config->control.pcfich_nregs;i++) {
		k=(kh+(i*config->nof_prb/2)*(NOF_RE_X_OSYMB/2))%modulo;
		config->control.pcfich[i] = lte_reg_get_k(k,0,config);
		if (!config->control.pcfich[i]) {
			if (config->verbose) {
				printf("Error allocating PCFICH: REG (%d,0) not found\n",k);
			}
			return -1;
		}
		if (config->control.pcfich[i]->assigned != 1) {
			if (config->verbose) {
				printf("Error allocating PCFICH: REG (%d,0) %s\n",k,reg_print_state(config->control.pcfich[i]));
			}
			return -1;
		} else {
			config->control.pcfich[i]->assigned = 2;
			if (config->verbose) {
				printf("Assigned PCFICH REG#%d (%d,0)\n",i,k);
			}
		}
	}
	return 0;
}

int lte_pcfich_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i;
	strcpy(ch->name,"PCFICH");

	config->control.pcfich_nregs = PCFICH_REG;

	if (lte_grid_init_reg_pcfich(config)) {
		return -1;
	}

	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		ch->symbol_mask[i] = 0x1;
		ch->nof_re_x_sf[i] = config->control.pcfich_nregs*NOF_RE_X_REG;
	}
	return 0;
}


int lte_pcfich_put(complex_t *pcfich, complex_t *output,
		struct lte_grid_config *config) {
	int i;

	for (i=0;i<config->control.pcfich_nregs;i++) {
		lte_reg_put(&pcfich[4*i],output,config->control.pcfich[i],config);
	}
	return i*NOF_RE_X_REG;
}


int lte_pcfich_get(complex_t *input,complex_t *pcfich,
		struct lte_grid_config *config) {
	int i;

	for (i=0;i<config->control.pcfich_nregs;i++) {
		lte_reg_get(input,&pcfich[4*i],config->control.pcfich[i],config);
	}
	return i*NOF_RE_X_REG;
}



#include <stdio.h>
#include <string.h>
#include "base.h"


int lte_phich_put(complex_t *phich, complex_t *output,
		struct lte_grid_config *config) {
	int i;

	for (i=0;i<config->control.phich_nregs;i++) {
		lte_reg_put(&phich[4*i],output,config->control.phich[i],config);
	}
	return i*NOF_RE_X_REG;
}


int lte_phich_get(complex_t *input,complex_t *phich,
		struct lte_grid_config *config) {
	int i;

	for (i=0;i<config->control.phich_nregs;i++) {
		lte_reg_get(input,&phich[4*i],config->control.phich[i],config);
	}
	return i*NOF_RE_X_REG;
}


int lte_grid_init_reg_phich_countfree(int symbol_id, struct lte_grid_config *config) {
	int p,r,n;
	n=0;
	for (p=0;p<config->nof_prb;p++) {
		for (r=0;r<MAX_REGS_X_PRB;r++) {
			if (config->control.regs[p][symbol_id][r].assigned==1) {
				n++;
			}
		}
	}
	return n;
}

struct lte_reg *lte_grid_init_reg_phich_ni(int symbol_id, int ni, struct lte_grid_config *config) {
	int p,r,nip;
	nip=0;
	for (p=0;p<config->nof_prb;p++) {
		for (r=0;r<lte_reg_num(symbol_id,config);r++) {
			if (config->control.regs[p][symbol_id][r].assigned==1) {
				if (ni==nip) {
					return &config->control.regs[p][symbol_id][r];
				}
				nip++;
			}
		}
	}
	return NULL;
}

/** 36.211 10.3 section 6.9.3
 *
 * Must be called after lte_grid_init_reg_pcfich()
 *
 */
int lte_grid_init_reg_phich(struct lte_grid_config *config) {
	int i,ni,li,n[3],p,r,nip,nreg,mi;

	memset(n,0,sizeof(int)*3);
	for (i=0;i<3;i++) {
		n[i] = lte_grid_init_reg_phich_countfree(i,config);
	}

	nreg=0;
	for (mi=0;mi<config->phich_ngroups;mi++) {
		for (i=0;i<3;i++) {
			li=config->phich_duration>1?i:0;
			ni=((config->cell_id*n[li]/n[0])+mi+i*n[li]/3) % n[li];
			config->control.phich[nreg] = lte_grid_init_reg_phich_ni(li, ni, config);
			if (!config->control.phich[nreg]) {
				printf("Error allocating PHICH: REG l=%d, ni=%d not found\n",li,ni);
				return -1;
			}
			if (config->verbose) {
				printf("Assigned PHICH REG#%d (%d,%d)\n",nreg,config->control.phich[nreg]->k0,li);
			}
			nreg++;
		}
	}
	for (i=0;i<nreg;i++) {
		config->control.phich[i]->assigned = 2;
	}
	return 0;
}

int lte_phich_init_params(struct lte_grid_config *config) {
	config->phich_duration = 1;
	config->phich_ngfactor = 1.0;
	config->phich_ngroups = (int) ((config->phich_ngfactor*config->nof_prb-1)/8+1);
	return 0;
}

int lte_phich_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i;
	strcpy(ch->name,"PHICH");

	if (lte_phich_init_params(config)) {
		return -1;
	}

	config->control.phich_nregs = config->phich_ngroups*3;

	if (lte_grid_init_reg_phich(config)) {
		return -1;
	}

	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		if (config->phich_duration == 1) {
			ch->symbol_mask[i] = 0x1;
		} else if (config->phich_duration == 3) {
			ch->symbol_mask[i] = 0x3;
		}
		ch->nof_re_x_sf[i] = config->control.phich_nregs*NOF_RE_X_REG;
	}
	return 0;
}


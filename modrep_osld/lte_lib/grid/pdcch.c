#include <stdio.h>
#include <string.h>
#include "base.h"
#include "params.h"


/** 36.211 10.3 section 6.8
 */
int lte_grid_init_reg_pdcch(struct lte_pdcch *ch, struct lte_grid_config *config) {
	int i,j,k,l;
	struct lte_reg *reg;

	ch->nof_regs = NOF_REGS_X_CCE * ch->nof_cce;
	ch->nof_re = NOF_RE_X_REG * ch->nof_regs;
	ch->nof_bits = ch->nof_re * 2;

	k=l=0;
	for (i=0;i<ch->nof_cce;i++) {
		for (j=0;j<NOF_REGS_X_CCE;j++) {
			do {
				reg = lte_reg_get_k(k, l, config);
				if (!reg) {
					printf("Error initiating PDCCH: REG (%d,%d) not found\n",k,l);
					break;
				}
				if (reg->assigned != 1) {
					l++;
					if (l==config->nof_control_symbols) {
						l=0;
						k++;
						if (k==config->nof_prb*NOF_RE_X_OSYMB) {
							printf("Can't find an empty REG\n");
							return -1;
						}
					}
				}
			}while(reg->assigned != 1);
			reg->assigned = 2;
			ch->cce[i].regs[j] = reg;
			if (config->verbose) {
				printf("Assigned PDCCH#%d REG#%d, CCE#%d (%d,%d)\n",ch->id,j,i,k,l);
			}
		}
	}
	return 0;
}

int lte_pdcch_init_params(struct lte_grid_config *config) {
	int i;
	char tmp[64];
	param_get_int_name("nof_pdcch",&config->nof_pdcch);
	if (config->nof_pdcch>MAX_PDCCH || config->nof_pdcch<0) {
		printf("Error only %d PDCCH are supported (%d)\n",MAX_PDCCH,config->nof_pdcch);
		return -1;
	}
	for (i=0;i<config->nof_pdcch;i++) {
		snprintf(tmp,64,"pdcch_format_%d",i);
		if (param_get_int_name(tmp,&config->pdcch[i].format)) {
			config->pdcch[i].format = 0;
		}
		config->pdcch[i].nof_cce = (0x1<<config->pdcch[i].format);
		config->pdcch[i].id=i;
	}

	return 0;
}

int lte_pdcch_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i,j;
	int nregs;
	strcpy(ch->name,"PDCCH");

	if (lte_pdcch_init_params(config)) {
		return -1;
	}

	if (!config->nof_pdcch) {
		return 0;
	}

	config->control.pdcch_nregs = config->control.total_nregs-config->control.pcfich_nregs-
			config->control.phich_nregs;

	for (i=0;i<config->nof_pdcch;i++) {
		if (lte_grid_init_reg_pdcch(&config->pdcch[i],config)) {
			return -1;
		}
	}

	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		for (j=0;j<config->nof_control_symbols;j++) {
			ch->symbol_mask[i] <<= 1;
			ch->symbol_mask[i] |= 0x1;
		}
		ch->nof_re_x_sf[i] = (config->control.total_nregs-config->control.phich_nregs-
				config->control.pcfich_nregs)*NOF_RE_X_REG;
	}
	return 0;
}

int lte_pdcch_get_bits(int ch_idx, struct lte_grid_config *config) {
	if (ch_idx<0 || ch_idx > config->nof_pdcch) {
		return -1;
	}
	return config->pdcch[ch_idx].nof_bits;
}

int lte_pdcch_get_re(int ch_idx, struct lte_grid_config *config) {
	if (ch_idx<0 || ch_idx > config->nof_pdcch) {
		return -1;
	}
	return config->pdcch[ch_idx].nof_re;
}

int lte_pdcch_put(complex_t *pdcch, complex_t *output, int channel_idx,
		struct lte_grid_config *config) {
	int i,c;

	if (channel_idx < 0 || channel_idx > config->nof_pdcch) {
		return -1;
	}

	for (c=0;c<config->pdcch[channel_idx].nof_cce;c++) {
		for (i=0;i<NOF_REGS_X_CCE;i++) {
			lte_reg_put(&pdcch[c*NOF_REGS_X_CCE+4*i],output,
					config->pdcch[channel_idx].cce[c].regs[i],config);
		}
	}
	return config->pdcch[channel_idx].nof_bits;
}


int lte_pdcch_get(complex_t *input, complex_t *pdcch, int channel_idx,
		struct lte_grid_config *config) {
	int i,c;

	if (channel_idx < 0 || channel_idx > config->nof_pdcch) {
		return -1;
	}

	for (c=0;c<config->pdcch[channel_idx].nof_cce;c++) {
		for (i=0;i<NOF_REGS_X_CCE;i++) {
			lte_reg_get(input,&pdcch[c*NOF_REGS_X_CCE+4*i],
					config->pdcch[channel_idx].cce[c].regs[i],config);
		}
	}
	return config->pdcch[channel_idx].nof_re;
}

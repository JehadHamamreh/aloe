#include <stdio.h>
#include <string.h>
#include "base.h"


int lte_reg_num(int sym_id, struct lte_grid_config *config) {
	struct lte_symbol symbol;
	symbol.subframe_id = 0;
	symbol.symbol_id = sym_id;
	if (lte_symbol_has_refsig_or_resv(&symbol, config)) {
		return 2;
	} else {
		return 3;
	}
}


int lte_reg_indices(int nreg, int kp, int sym_id, int maxreg, int vo,
		struct lte_reg *reg, struct lte_grid_config *config) {
	int i,k0,j,z;
	switch(maxreg) {
	case 2:
		k0=nreg*6;
		/* there are two references in the middle */
		j=z=0;
		for (i=0;i<vo;i++) {
			reg->k[j] = kp+k0+i;
			j++;
		}
		reg->k_ref[0] = kp+k0+vo;
		for (i=0;i<2;i++) {
			reg->k[j] = kp+k0+i+vo+1;
			j++;
		}
		reg->k_ref[1] = kp+k0+vo+3;
		z=j;
		for (i=0;i<4-z;i++) {
			reg->k[j] = kp+k0+vo+3+i+1;
			j++;
		}
		if (j!=4) {
			printf("Something went wrong: expected 2 references\n");
			return -1;
		}

		reg->k0=kp+k0;
		reg->k3=kp+k0+6;
		break;
	case 3:
		k0=nreg*4;
		/* there is no reference */
		for (i=0;i<4;i++) {
			reg->k[i]=kp+k0+i;
		}
		for (i=0;i<2;i++) {
			reg->k_ref[i] = -1;
		}
		reg->k0=kp+k0;
		reg->k3=kp+k0+4;;
		break;
	}
	return 0;
}


int lte_grid_init_reg(struct lte_grid_config *config, int nof_ctrl_symbols) {
	int i,j,n,p,nr;
	int vo=config->cell_id % 3;

	config->control.total_nregs=0;
	for (i=0;i<nof_ctrl_symbols;i++) {
		config->control.total_nregs += config->nof_prb*lte_reg_num(i,config);
	}

	nr=0;
	for (i=0;i<nof_ctrl_symbols;i++) {
		n=lte_reg_num(i,config);
		for (p=0;p<config->nof_prb;p++) {
			for (j=0;j<n;j++) {
				if (lte_reg_indices(j,p*NOF_RE_X_OSYMB,i,n,vo,&config->control.regs[p][i][j],config)) {
					printf("Error initializing REGs\n");
					return -1;
				}
				config->control.regs[p][i][j].symbol = i;
				config->control.regs[p][i][j].id=j;
				config->control.regs[p][i][j].assigned = 1;
				if (config->verbose) {
					printf("Available REG #%d: %d:%d:%d (k0=%d,k3=%d)\n",nr,i,p,j,
							config->control.regs[p][i][j].k0,config->control.regs[p][i][j].k3);
				}
				nr++;
			}
		}
	}
	return 0;
}




struct lte_reg *lte_reg_get_k(int k, int l, struct lte_grid_config *config) {
	int i;
	int prb = k/NOF_RE_X_OSYMB;
	int k0 = k;
	if (l<0 || l>MAX_CTRL_SYMB) {
		return NULL;
	}
	for (i=0;i<MAX_REGS_X_PRB;i++) {
		if (config->control.regs[prb][l][i].k0<=k0 &&
				k0<config->control.regs[prb][l][i].k3) {
			return &config->control.regs[prb][l][i];
		}
	}
	return NULL;
}

int lte_reg_put_sf_pos(int k, int l,struct lte_grid_config *config) {
	return l*config->fft_size+config->pre_guard+k;
}

/* Puts a zero in the reference signal */
int lte_reg_put(complex_t *input, complex_t *output, struct lte_reg *reg,struct lte_grid_config *config) {
	int i;
	for (i=0;i<NOF_RE_X_REG;i++) {
		output[lte_reg_put_sf_pos(reg->k[i],reg->symbol,config)] = input[i];
	}
	for (i=0;i<2;i++) {
		if (reg->k_ref[i] >= 0) {
			output[lte_reg_put_sf_pos(reg->k_ref[i],reg->symbol,config)] = 0;
		}
	}
	return 4;
}

int lte_reg_get(complex_t *input, complex_t *output, struct lte_reg *reg,struct lte_grid_config *config) {
	int i;
	for (i=0;i<NOF_RE_X_REG;i++) {
		output[i] = input[lte_reg_put_sf_pos(reg->k[i],reg->symbol,config)];
	}
	return 4;
}



const char *reg_print_state(struct lte_reg *reg) {
	if (!reg->assigned) {
		return "Not Available";
	} else if (reg->assigned == 1) {
		return "Available";
	} else if (reg->assigned == 2) {
		return "Assigned";
	} else {
		return "Invalid";
	}
}


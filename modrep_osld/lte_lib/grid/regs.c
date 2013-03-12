#include <stdio.h>
#include <string.h>
#include "base.h"


int lte_reg_num(int sym_id, struct lte_grid_config *config) {
	switch(sym_id) {
	case 0:
		return 2;
	case 1:
		if (config->nof_ports<4) {
			return 3;
		} else {
			return 3;
		}
	case 2:
		return 3;
	case 3:
		if (config->nof_osymb_x_subf == NOF_OSYMB_X_SLOT_NORMAL) {
			return 3;
		} else {
			return 2;
		}
	}
	return 0;
}


int lte_reg_indices(int nreg, int kp, int sym_id, struct lte_reg *reg, struct lte_grid_config *config) {
	int i,k0,j,z;
	int maxreg;
	maxreg = lte_reg_num(sym_id,config);
	if (nreg>=maxreg) {
		return -1;
	}
	switch(maxreg) {
	case 2:
		k0=nreg*6;
		/* there are two references in the middle */
		j=z=0;
		for (i=0;i<6;i++) {
			if (lte_re_has_refsig_or_resv(kp+k0+i,sym_id,config)) {
				if (z >= 2) {
					printf("Something went worng, expected 2 reference in the reg\n");
					return -1;
				}
				reg->k_ref[z] = kp+k0+i;
				z++;
			} else {
				if (j >= 4) {
					printf("Something went worng, expected 4 free RE in the reg\n");
					return -1;
				}
				reg->k[j]=kp+k0+i;
				j++;
			}
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
}

int lte_reg_put_sf_pos(int k, int l,struct lte_grid_config *config) {
	return l*config->fft_size+config->pre_guard+k;
}

/* Puts a zero in the reference signal */
int lte_reg_put(complex_t *input, complex_t *output, struct lte_reg *reg,struct lte_grid_config *config) {
	int i,k;
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
	int i,k;
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


int lte_grid_init_reg(struct lte_grid_config *config) {
	int i,j,n,p,nr;

	config->control.total_nregs=0;
	for (i=0;i<config->nof_control_symbols;i++) {
		config->control.total_nregs += config->nof_prb*lte_reg_num(i,config);
	}

	nr=0;
	for (i=0;i<config->nof_control_symbols;i++) {
		for (p=0;p<config->nof_prb;p++) {
			n=lte_reg_num(i,config);
			for (j=0;j<n;j++) {
				if (lte_reg_indices(j,p*NOF_RE_X_OSYMB,i,&config->control.regs[p][i][j],config)) {
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



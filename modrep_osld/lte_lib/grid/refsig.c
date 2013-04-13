#include <string.h>
#include <math.h>

#include "base.h"
#include "sequences.h"

/* reference or reserved for any port if port_id==-1, or for the given port otherwise */
int lte_refsig_symbolmask(int port_id, struct lte_grid_config *config) {
	int symbol_mask,out;

	if (config->nof_ports == 4 && port_id >= 2) {
		symbol_mask = 0x2;
	} else {
		symbol_mask = 0x1 | (0x1<<(config->nof_osymb_x_subf/2-3));
	}
	if (config->nof_ports == 4 && port_id == -1) {
		symbol_mask |= 0x2;
	}

	out = (symbol_mask<<config->nof_osymb_x_subf/2) | symbol_mask;

	return out;
}
/* reference or reserved for any port */
int lte_refsig_or_resv_voffset(int symbol_id, struct lte_grid_config *config) {
	int vo = config->cell_id % 6;
	if (config->nof_ports == 1 && (symbol_id%(config->nof_osymb_x_subf/2)!=0)) {
		return (config->cell_id+3)%6;
	} else {
		return vo;
	}
}

/* reference or reserved for any port */
int lte_symbol_has_refsig_or_resv(struct lte_symbol *symbol, struct lte_grid_config *config) {
	if (symbol->subframe_id == 0 &&
			(symbol->symbol_id == config->nof_osymb_x_subf/2 || symbol->symbol_id == config->nof_osymb_x_subf/2+1)) {
		return 4;
	}
	if (!(lte_refsig_symbolmask(-1,config) & (0x1<<symbol->symbol_id))) {
		return 0;
	}
	if (config->nof_ports == 1 && symbol->symbol_id > 0) {
		if (config->nof_osymb_x_subf == NOF_OSYMB_X_SLOT_EXT && symbol->symbol_id == config->nof_osymb_x_subf/2-3) {
			return 4;
		} else {
			return 2;
		}
	} else {
		return 4;
	}
}

/* reference at a given port*/
int lte_symbol_has_refsig(int port_id, int symbol_id, struct lte_grid_config *config) {

	return (lte_refsig_symbolmask(port_id,config) & (0x1<<symbol_id));
}

/* reference or reserved for any port */
int lte_re_has_refsig_or_resv(int k, int symbol_id, struct lte_grid_config *config) {
	struct lte_symbol symbol;
	int nof_ref;
	symbol.subframe_id = 0;
	symbol.symbol_id = symbol_id;
	nof_ref = lte_symbol_has_refsig_or_resv(&symbol,config);
	if (!nof_ref) {
		return 0;
	}
	return ((k % (NOF_RE_X_OSYMB/nof_ref)) == lte_refsig_or_resv_voffset(symbol_id,config));
}


int lte_refsig_init(int port_id, struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i;
	strcpy(ch->name,"REFSIG");
	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		ch->symbol_mask[i] = lte_refsig_symbolmask(port_id,config);
		if (port_id < 4) {
			ch->nof_re_x_sf[i] = 8*config->nof_prb;
		} else {
			ch->nof_re_x_sf[i] = 4*config->nof_prb;
		}
	}
	return 0;
}



int lte_refsig_l(int symbol_id, struct lte_grid_config *config) {
	int l;
	if (symbol_id == 0 || symbol_id == config->nof_osymb_x_subf/2) {
		l=0;
	} else if (symbol_id == config->nof_osymb_x_subf/2-3
			|| symbol_id == config->nof_osymb_x_subf-3) {
		l=1;
	} else {
		l=2;
	}
	return l;
}

int lte_refsig_v(int port_id, int ns, int symbol_id, struct lte_grid_config *config) {
	int v=-1;
	switch(port_id) {
	case 0:
		if (symbol_id == 0 || symbol_id == config->nof_osymb_x_subf/2) {
			v=0;
		} else {
			v=3;
		}
		break;
	case 1:
		if (symbol_id == 0 || symbol_id == config->nof_osymb_x_subf/2) {
			v=3;
		} else {
			v=0;
		}
		break;
	case 2:
		v=3*(ns%2);
		break;
	case 3:
		v=3+3*(ns%2);
		break;
	}
	return v;
}

int lte_refsig_k(int m, int v, struct lte_grid_config *config) {
	return 6*m+((v+(config->cell_id%6))%6);
}

inline int lte_refsig_mp(int m, struct lte_grid_config *config) {
	return m+MAX_NPRB-config->nof_prb;
}


/** 36.211 10.3 section 6.10.1.2
 */
int lte_refsig_put(refsignal_t *refsignal, complex_t *output,
		struct lte_symbol *location, struct lte_grid_config *config) {

	int m,ns,l,mp;

	if (!lte_symbol_has_refsig(refsignal->port_id, location->symbol_id,config)) {
		return 0;
	}

	l = lte_refsig_l(location->symbol_id,config);
	ns = lte_get_ns(location,config);

	for (m=0;m<2*config->nof_prb;m++) {
		mp=lte_refsig_mp(m,config);
		output[refsignal->k[l][ns][mp]] = refsignal->signal[l][ns][mp];
	}
	return m;
}

int lte_refsig_get(complex_t *input, refsignal_t *refsignal,
		struct lte_symbol *location, struct lte_grid_config *config) {

	int m,ns,l;

	if (!lte_symbol_has_refsig(refsignal->port_id, location->symbol_id,config)) {
		return 0;
	}

	l = lte_refsig_l(location->symbol_id,config);
	ns = lte_get_ns(location,config);

	for (m=0;m<2*config->nof_prb;m++) {
		refsignal->signal[l][ns][lte_refsig_mp(m,config)] =
				input[refsignal->k[l][ns][lte_refsig_mp(m,config)]];
	}
	return m;
}



/** 36.211 10.3 section 6.10.1.2
 */
void generate_cref_k(refsignal_t *refsignal, struct lte_grid_config *config) {

	int m,v,k;
	int ns,l,lp[REF_L];

	lp[0] = 0;
	lp[1] = config->nof_osymb_x_subf/2-3;
	lp[2] = 1;

	for (l=0;l<REF_L;l++) {
		for (ns=0;ns<REF_NS;ns++) {
			v = lte_refsig_v(refsignal->port_id, ns, lp[l], config);
			for (m=0;m<2*config->nof_prb;m++) {
				k = lte_refsig_k(m,v,config)+config->pre_guard;
				refsignal->k[l][ns][lte_refsig_mp(m,config)] = k;
			}
		}
	}
}

unsigned int c[2*REF_M];

/** 36.211 10.3 section 6.10.1.1
 *
 */
void generate_cref_signal(refsignal_t *refsignal, struct lte_grid_config *config) {
	unsigned int c_init;
	int ns,l,lp[REF_L];
	int N_cp;
	int i;

	lp[0] = 0;
	lp[1] = config->nof_osymb_x_subf/2-3;
	lp[2] = 1;

	if(config->nof_osymb_x_subf == NOF_OSYMB_X_SLOT_NORMAL) {
		N_cp=1;
	} else {
		N_cp=0;
	}

	for (l=0;l<REF_L;l++) {
		for (ns=0;ns<REF_NS;ns++) {
			c_init = 1024*(7*(ns+1)+lp[l]+1)*(2*config->cell_id+1)+2*config->cell_id+N_cp;
			generate_prs_c(c_init,2*REF_M,c);
			for(i=0; i<REF_M; i++) {
				if (config->debug) {
					__real__ refsignal->signal[l][ns][i] = 3.0;
					__imag__ refsignal->signal[l][ns][i] = 3.0;
				} else {
					__real__ refsignal->signal[l][ns][i] = (1 - 2*(float)c[2*i])/sqrt(2);
					__imag__ refsignal->signal[l][ns][i] = (1 - 2*(float)c[2*i+1])/sqrt(2);
				}
			}
		}
	}
}

void generate_cref(refsignal_t *refsignal, struct lte_grid_config *config) {
	generate_cref_signal(refsignal,config);
	generate_cref_k(refsignal,config);
}

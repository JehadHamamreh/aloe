#include <stdio.h>
#include <string.h>
#include "base.h"
#include "params.h"


/* RA is RBG based contiguous and assumed to be constant during one frame.
 * However, it is possible to call lte_pdsch_setup_rbgmask every sub-frame
 * and obtain time-variant RA.
 */

int lte_pdsch_fill_rbg(int *rbg_vector, struct lte_pdsch *ch, struct lte_grid_config *config) {
	int i;
	for (i=0;i<config->nof_prb/RBG_SZ(config->nof_prb);i++) {
		if (ch->rbg_mask & (0x1<<i)) {
			rbg_vector[i] = 1;
		} else {
			rbg_vector[i] = 0;
		}
	}
	return i;
}


/* FIXME: NOF_PRB must be multiple of RBG_SZ */
void lte_pdsch_setup_rbgmask(struct lte_pdsch *ch,struct lte_grid_config *config) {
	int i,j;
	ch->nof_rbg = 0;
	for (i=0;i<config->nof_prb/RBG_SZ(config->nof_prb);i++) {
		if (ch->rbg_mask & (0x1<<i)) {
			for(j=0;j<RBG_SZ(config->nof_prb);j++) {
				ch->rbg[ch->nof_rbg].prb_idx[j] = RBG_SZ(config->nof_prb)*i+j;
				ch->nof_rbg++;
			}
		}
	}
	ch->nof_rb = RBG_SZ(config->nof_prb)*ch->nof_rbg;
	/* Assume frequency-selective allocation only */
	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		ch->nof_re[i] = ch->nof_rb*(config->phch[CH_PDSCH].nof_re_x_sf[i]/config->nof_prb);
	}
}

int lte_pdsch_init_params_ch(int ch_id, struct lte_grid_config *config) {
	char tmp[64];

	snprintf(tmp,64,"pdsch_rbgmask_%d",ch_id);
	if (param_get_int_name(tmp,&config->pdsch[ch_id].rbg_mask)) {
		if (config->nof_pdsch == 1) {
			config->pdsch[ch_id].rbg_mask = 0xFFFFFFFF;
		} else {
			config->pdsch[ch_id].rbg_mask = 1<<ch_id;
		}
	}
	return 0;
}

int lte_pdsch_init_params(struct lte_grid_config *config) {
	int i;


	param_get_int_name("nof_pdsch",&config->nof_pdsch);
	if (config->nof_pdsch>MAX_PDSCH) {
		printf("Error only %d PDSCH are supported (%d)\n",MAX_PDCCH,config->nof_pdsch);
		return -1;
	}
	for (i=0;i<config->nof_pdsch;i++) {
		lte_pdsch_init_params_ch(i,config);
	}
	return 0;
}

const int pdsch_ref_prb[MAX_PORTS][2] = {
		{4,6},
		{8,12},
		{4,8},
		{4,16}
};

const int pdsch_used_symbols[10] = {
		6,0,0,0,0,2,0,0,0,0
};

int lte_pdsch_re_x_prb(int subframe_idx, struct lte_grid_config *config) {
	int i,j;
	switch(config->nof_ports) {
	case 1: i=0; break;
	case 2: i=1; break;
	case 4: i=2; break;
	default: return -1;
	}
	if (!subframe_idx) {
		j=0;
	} else {
		j=1;
	}
	return NOF_RE_X_OSYMB*(config->nof_osymb_x_subf-
			config->nof_control_symbols-pdsch_used_symbols[subframe_idx])-
			pdsch_ref_prb[i][j];
}

int lte_pdsch_init_sf(int subframe_id, struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i,j;

	if (subframe_id<0 || subframe_id > NOF_SUBFRAMES_X_FRAME) {
		return -1;
	}
	i = subframe_id;
	/* transmit all symbols... */
	ch->symbol_mask[i] = ((0x1<<(config->nof_osymb_x_subf+1))-1);

	/* ... except control symbols */
	for (j=0;j<config->nof_control_symbols;j++) {
		ch->symbol_mask[i] ^= (0x1<<j);
	}
	/*... if 6 prb*/
	if (config->nof_prb == 6) {
		/* switch off in pbch */
		if (i==0) {
			for (j=config->nof_osymb_x_subf/2;j<config->nof_osymb_x_subf/2+4;j++) {
				ch->symbol_mask[i] ^= (0x1<<j);
			}
		}
		/* and pss/sss */
		if (i==0 || i==5) {
			ch->symbol_mask[i] ^= 0x1<<(config->nof_osymb_x_subf/2-1);
			ch->symbol_mask[i] ^= 0x1<<(config->nof_osymb_x_subf/2-2);
		}
	}
	ch->nof_re_x_sf[i] = config->nof_prb*lte_pdsch_re_x_prb(i,config);
	if (ch->nof_re_x_sf[i]<0) {
		printf("PDSCH: Error computing RE x RB\n");
		return -1;
	}
	return 0;
}

int lte_pdsch_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i,j;
	struct lte_symbol symbol;
	strcpy(ch->name,"PDSCH");

	if (lte_pdsch_init_params(config)) {
		return -1;
	}

	if (!config->nof_pdsch) {
		return 0;
	}

	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		lte_pdsch_init_sf(i, ch, config);
	}

	for (i=0;i<config->nof_pdsch;i++) {
		lte_pdsch_setup_rbgmask(&config->pdsch[i],config);
		for (j=0;j<NOF_SUBFRAMES_X_FRAME;j++) {
			if (config->verbose) {
				printf("PDSCH#%d has %d RB and %d RE in subframe %d\n",i,config->pdsch[i].nof_rb,
						config->pdsch[i].nof_re[j],j);
			}
		}
	}
	return 0;
}

int lte_pdsch_get_re(int channel_idx, int subframe_id, struct lte_grid_config *config) {
	if (channel_idx < 0 || channel_idx > config->nof_pdsch) {
		return -1;
	}
	if (subframe_id<0 || subframe_id > NOF_SUBFRAMES_X_FRAME) {
		return -1;
	}
	return config->pdsch[channel_idx].nof_re[subframe_id];
}


/*
 * Symbol is either completely free or used by reference signals.
 */
int lte_pdsch_put(complex_t *pdsch, complex_t *output, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int rp,wp;
	int i,j;
	int nof_ref_x_rb;

	if (!lte_symbol_has_ch(location,&config->phch[CH_PDSCH])) {
		return 0;
	}
	nof_ref_x_rb = lte_symbol_has_refsig_or_resv(location,config);
	rp=0;
	for (i=0;i<config->pdsch[channel_idx].nof_rbg;i++) {
		for (j=0;j<RBG_SZ(config->nof_prb);j++) {
			wp = NOF_RE_X_OSYMB*config->pdsch[channel_idx].rbg[i].prb_idx[j];
			if (nof_ref_x_rb) {
				lte_ch_put_ref(&pdsch[rp],&output[wp],1,
						lte_refsig_or_resv_voffset(location->symbol_id, config),
						NOF_RE_X_OSYMB/nof_ref_x_rb);
				rp+=NOF_RE_X_OSYMB-nof_ref_x_rb*config->nof_ports;
			} else {
				lte_ch_cp_noref(&pdsch[rp],&output[wp],
						1);
				rp+=NOF_RE_X_OSYMB;
			}
		}
	}
	return rp;
}


/*
 * Symbol is either completely free or used by reference signals.
 */
int lte_pdsch_get(complex_t *input, complex_t *pdsch, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int rp,wp;
	int i,j;
	int nof_ref_x_rb;

	if (!lte_symbol_has_ch(location,&config->phch[CH_PDSCH])) {
		return 0;
	}
	nof_ref_x_rb = lte_symbol_has_refsig_or_resv(location,config);
	wp=0;
	for (i=0;i<config->pdsch[channel_idx].nof_rbg;i++) {
		for (j=0;j<RBG_SZ(config->nof_prb);j++) {
			rp = NOF_RE_X_OSYMB*config->pdsch[channel_idx].rbg[i].prb_idx[j];
			if (nof_ref_x_rb) {
				lte_ch_get_ref(&input[rp], &pdsch[wp],1,
						lte_refsig_or_resv_voffset(location->symbol_id, config),
						NOF_RE_X_OSYMB/nof_ref_x_rb);
				wp+=NOF_RE_X_OSYMB-nof_ref_x_rb*config->nof_ports;
			} else {
				lte_ch_cp_noref(&input[rp], &pdsch[wp],
						1);
				wp+=NOF_RE_X_OSYMB;
			}
		}
	}
	return wp;
}


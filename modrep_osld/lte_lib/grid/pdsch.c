#include <stdio.h>
#include <string.h>
#include "base.h"
#include "params.h"


/* RA is RBG based contiguous and assumed to be constant during one frame.
 * However, it is possible to call lte_pdsch_setup_rbgmask every sub-frame
 * and obtain time-variant RA.
 */



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

int lte_pdsch_init_params(struct lte_grid_config *config) {
	int i;
	char tmp[64];

	if (param_get_int_name("nof_pdsch",&config->nof_pdsch)) {
		config->nof_pdsch = 1;
	}
	if (config->nof_pdsch>MAX_PDSCH) {
		printf("Error only %d PDSCH are supported (%d)\n",MAX_PDCCH,config->nof_pdsch);
		return -1;
	}
	for (i=0;i<config->nof_pdsch;i++) {
		snprintf(tmp,64,"pdsch_rbgmask_%d",i);
		if (param_get_int_name(tmp,&config->pdsch[i].rbg_mask)) {
			if (config->nof_pdsch == 1) {
				config->pdsch[i].rbg_mask = 0xFFFFFFFF;
			} else {
				config->pdsch[i].rbg_mask = 1<<i;
			}
		}
	}
	return 0;
}

int lte_pdsch_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i,j;
	strcpy(ch->name,"PDSCH");

	if (lte_pdsch_init_params(config)) {
		return -1;
	}

	for (i=0;i<NOF_SUBFRAMES_X_FRAME;i++) {
		/* transmit on those symbols not used by pdcch */
		ch->symbol_mask[i] = ((0x1<<(config->nof_osymb_x_subf+1))-1)^config->phch[CH_PDCCH].symbol_mask[i];
		/* if 6 prb, switch off on pss/sss/pbch symbols either */
		if (config->nof_prb == 6) {
			ch->symbol_mask[i] ^= config->phch[CH_PBCH].symbol_mask[i];
			ch->symbol_mask[i] ^= config->phch[CH_PSS].symbol_mask[i];
			ch->symbol_mask[i] ^= config->phch[CH_SSS].symbol_mask[i];
		}
		ch->nof_re_x_sf[i] = config->nof_prb*config->nof_osymb_x_subf*NOF_RE_X_OSYMB;

		/* remove re used by control or synch channels */
		for (j=0;j<CH_PDSCH;j++) {
			ch->nof_re_x_sf[i] -= config->phch[j].nof_re_x_sf[i];
		}
		/* remove re used by reference signals */
		for (j=0;j<config->nof_ports;j++) {
			ch->nof_re_x_sf[i] -= config->refsig_cfg[j].nof_re_x_sf[i];
		}

		/* and remove unused re if one port only */
		if (config->nof_ports == 1) {
			ch->nof_re_x_sf[i] -= 2*config->nof_prb;
			/* in subframe 0, bch has 6 refs unused */
			if (i==0) {
				ch->nof_re_x_sf[i] -= 6*config->nof_prb;
			}
		} else if (config->nof_ports == 2) {
			if (i==0) {
				ch->nof_re_x_sf[i] -= 4*config->nof_prb;
			}
		}
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


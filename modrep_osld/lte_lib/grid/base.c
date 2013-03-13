/*
 * Copyright (c) 2012,
 * Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 *
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "base.h"
#include "tables.h"
#include "params.h"


int lte_get_fft_size_idx(int fft_size) {
	switch(fft_size) {
	case 128: return 0;
	case 256: return 1;
	case 512: return 2;
	case 1024: return 3;
	case 1536: return 4;
	case 2048: return 5;
	default: return -1;
	}
}

int lte_get_modulation_format(int mcs) {
	if (mcs<0 || mcs > 31) {
		return -1;
	}
	return Qm_71711[mcs];
}

int lte_get_tbs(int mcs, int nrb) {
	if (mcs<0 || mcs > 31) {
		return -1;
	}
	if (nrb<0 || nrb > 110) {
		return -1;
	}
	return TBS_71721[mcs][nrb];
}

int lte_get_ns(struct lte_symbol *location, struct lte_grid_config *config) {
	return location->subframe_id*2+(location->symbol_id<config->nof_osymb_x_subf/2?1:0);
}


int tb_2_cb(int x) {
	return 3*(x+LONG_CRC)+12;
}

int lte_get_cbits(int mcs, int nrb) {
	int tbs;
	if (mcs<0 || mcs > 31) {
		return -1;
	}
	if (nrb<0 || nrb > 110) {
		return -1;
	}
	tbs=TBS_71721[mcs][nrb];
	return tb_2_cb(tbs);
}

int lte_sf_has_ch(int subframe_id, struct lte_phch_config *ch) {
	return ch->symbol_mask[subframe_id]!=0;
}

int lte_symbol_has_ch(struct lte_symbol *symbol, struct lte_phch_config *ch) {
	return (0x1<<symbol->symbol_id)&ch->symbol_mask[symbol->subframe_id];
}

int lte_ch_init(struct lte_phch_config *ch, int code, struct lte_grid_config *config) {
	int n;
	memset(ch->symbol_mask,0,sizeof(int)*NOF_SUBFRAMES_X_FRAME);
	memset(ch->nof_re_x_sf,0,sizeof(int)*NOF_SUBFRAMES_X_FRAME);
	switch(code) {
	case CH_PDSCH:
		n=lte_pdsch_init(ch,config);
		break;
	case CH_PDCCH:
		n=lte_pdcch_init(ch,config);
		break;
	case CH_PCFICH:
		n=lte_pcfich_init(ch,config);
		break;
	case CH_PHICH:
		n=lte_phich_init(ch,config);
		break;
	case CH_PBCH:
		n=lte_pbch_init(ch,config);
		break;
	case CH_PSS:
		n=lte_pss_init(ch,config);
		break;
	case CH_SSS:
		n=lte_sss_init(ch,config);
		break;
	}
	return n;
}

int lte_ch_get_re(int ch_id, int ch_type, int subframe_idx, struct lte_grid_config *config) {
	if (subframe_idx < 0 || subframe_idx > NOF_SUBFRAMES_X_FRAME) {
		return -1;
	}
	if (ch_type != CH_REF) {
		if (ch_type<0 || ch_type > NOF_PHCH) {
			return -1;
		}
	}
	switch(ch_type) {
	case CH_PDSCH:
		return lte_pdsch_get_re(ch_id,subframe_idx,config);
		break;
	case CH_PDCCH:
		return lte_pdcch_get_re(ch_id,config);
		break;
	case CH_REF:
		if (ch_id<0 || ch_id > config->nof_ports) {
			return -1;
		}
		return config->refsig_cfg[ch_id].nof_re_x_sf[subframe_idx];
		break;
	default:
		return config->phch[ch_type].nof_re_x_sf[subframe_idx];
		break;
	}
}

int lte_ch_put_symbol(complex_t *input, complex_t *out_symbol, int ch_type, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int n;
	if (ch_type<0 || ch_type > NOF_PHCH) {
		return -1;
	}

	switch(ch_type) {
	case CH_PDSCH:
		n = lte_pdsch_put(input,&out_symbol[config->pre_guard],channel_idx,location,config);
		break;
	case CH_PDCCH:
		n = lte_pdcch_put(input,out_symbol,channel_idx,config);
		break;
	case CH_PCFICH:
		n = lte_pcfich_put(input,out_symbol,config);
		break;
	case CH_PHICH:
		n = lte_phich_put(input,out_symbol,config);
		break;
	case CH_PBCH:
		n = lte_pbch_put(input,&out_symbol[config->pre_guard],location,config);
		break;
	}
	return n;
}

int lte_ch_put_sf(complex_t *input, complex_t *out_symbol, int ch_type, int channel_idx,
		int subframe_id, struct lte_grid_config *config) {
	int i,rp,n;
	struct lte_symbol symbol;
	rp=0;

	symbol.subframe_id = subframe_id;
	switch(ch_type) {
	case CH_PDSCH:
	case CH_PBCH:
		for (i=0;i<config->nof_osymb_x_subf;i++) {
			symbol.symbol_id = i;
			n = lte_ch_put_symbol(&input[rp],&out_symbol[i*config->fft_size],
					ch_type,channel_idx,&symbol,config);
			if (n < 0) {
				return -1;
			}
			rp += n;
		}
		break;
	default:
		lte_ch_put_symbol(input,out_symbol,
				ch_type,channel_idx,&symbol,config);
		break;
	}
	return rp;
}


int lte_ch_get_symbol(complex_t *in_symbol, complex_t *output, int ch_type, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int n;
	if (ch_type<0 || ch_type > NOF_PHCH) {
		return -1;
	}

	switch(ch_type) {
	case CH_PDSCH:
		n = lte_pdsch_get(&in_symbol[config->pre_guard],output,channel_idx,location,config);
		break;
	case CH_PDCCH:
		n = lte_pdcch_get(in_symbol,output,channel_idx,config);
		break;
	case CH_PCFICH:
		n = lte_pcfich_get(in_symbol,output,config);
		break;
	case CH_PHICH:
		n = lte_phich_get(in_symbol,output,config);
		break;
	case CH_PBCH:
		n = lte_pbch_get(&in_symbol[config->pre_guard],output,location,config);
		break;
	}
	return n;
}

int lte_ch_get_sf(complex_t *in_symbol, complex_t *output, int ch_type, int channel_idx,
		int subframe_id, struct lte_grid_config *config) {
	int i,wp,n;
	struct lte_symbol symbol;
	wp=0;

	symbol.subframe_id = subframe_id;
	switch(ch_type) {
	case CH_PDSCH:
	case CH_PBCH:
		for (i=0;i<config->nof_osymb_x_subf;i++) {
			symbol.symbol_id = i;
			n = lte_ch_get_symbol(&in_symbol[i*config->fft_size],&output[wp],
					ch_type,channel_idx,&symbol,config);
			if (n < 0) {
				return -1;
			}
			wp += n;
		}
		break;
	default:
		wp=lte_ch_get_symbol(in_symbol,output,
				ch_type,channel_idx,&symbol,config);
		break;
	}
	return wp;
}

void lte_set_guard_symbol(complex_t *output, struct lte_grid_config *config) {
	memset(output,0,sizeof(complex_t)*config->pre_guard);
	memset(&output[config->fft_size-config->post_guard],
			0,sizeof(complex_t)*config->post_guard);
}

void lte_set_guard_sf(complex_t *output, struct lte_grid_config *config) {
	int i;
	for (i=0;i<config->nof_osymb_x_subf;i++) {
		lte_set_guard_symbol(&output[i*config->fft_size],config);
	}
}

void lte_set_refsign_sf(refsignal_t *ref, complex_t *output, int subframe_id, struct lte_grid_config *config) {
	int i;
	struct lte_symbol symbol;
	symbol.subframe_id = subframe_id;
	for (i=0;i<config->nof_osymb_x_subf;i++) {
		symbol.symbol_id = i;
		lte_refsig_put(ref,&output[i*config->fft_size+config->pre_guard],
				&symbol,config);
	}
}


int lte_grid_init_params(struct lte_grid_config *config) {
	int n=0;
	int i,j;
	if (!param_get_int_name("fft_size",&config->fft_size)) {
		n++;
	}
	if (!param_get_int_name("nof_prb",&config->nof_prb)) {
		n++;
	}
	if (!param_get_int_name("debug",&config->debug)) {
		n++;
	} else {
		config->debug=0;
	}
	if (!param_get_int_name("verbose",&config->verbose)) {
		n++;
	}
	if (!param_get_int_name("nof_osymb_x_subf",&config->nof_osymb_x_subf)) {
		n++;
	}
	if (!param_get_int_name("nof_ports",&config->nof_ports)) {
		n++;
	}
	if (!param_get_int_name("cfi",&config->cfi)) {
		n++;
	}
	if (!param_get_int_name("cell_id",&config->cell_id)) {
		n++;
	}
	config->nof_re_symb = config->nof_prb*NOF_RE_X_OSYMB;
	config->pre_guard = (config->fft_size-config->nof_re_symb)/2;
	config->post_guard = config->pre_guard;

	if (config->nof_prb<6 || config->nof_prb>110) {
		printf("Number of PRB must be between 0 and 110 (nof_prb=%d)\n",config->nof_prb);
		return -1;
	}
	if (config->cfi == -1) {
		config->nof_control_symbols = 0;
	} else {
		if (config->nof_prb<=10) {
			config->nof_control_symbols = config->cfi+1;
		} else {
			config->nof_control_symbols = config->cfi;
		}
	}
	return n;
}

int lte_grid_init_channels(struct lte_grid_config *config) {
	int i;
	for (i=0;i<config->nof_ports;i++) {
		lte_refsig_init(i,&config->refsig_cfg[i],config);
	}
	for (i=0;i<NOF_PHCH;i++) {
		if (lte_ch_init(&config->phch[i], i, config)) {
			printf("Error initiating channel %s\n",config->phch[i].name);
			return -1;
		}
	}
	return 0;
}

int lte_grid_init(struct lte_grid_config *config) {
	if (lte_grid_init_params(config)==-1) {
		return -1;
	}
	if (lte_grid_init_reg(config)) {
		printf("Error allocating REGs\n");
		return -1;
	}
	if (lte_grid_init_channels(config)) {
		return -1;
	}
	return 0;
}

void lte_ch_put_ref_one(complex_t *input, complex_t *output, int data_len) {
	output[0] = 0;
	memcpy(&output[1],input,data_len*sizeof(complex_t));
}

/* This function puts zeros every 3 re */
void lte_ch_put_ref(complex_t *input, complex_t *output,int nof_prb, int offset, int ref_interval) {
	int i,rp;

	memcpy(output,input,offset*sizeof(complex_t));
	rp=offset;
	for (i=0;i<(NOF_RE_X_OSYMB/ref_interval)*nof_prb-1;i++) {
		lte_ch_put_ref_one(&input[rp],&output[ref_interval*i+offset],ref_interval-1);
		rp+=(ref_interval-1);
	}
	if (ref_interval-offset-1 > 0) {
		lte_ch_put_ref_one(&input[rp],&output[ref_interval*i+offset],ref_interval-offset-1);
	}
}

void lte_ch_cp_noref(complex_t *input, complex_t *output,int nof_prb) {
	int i;
	for (i=0;i<nof_prb;i++) {
		memcpy(&output[i*NOF_RE_X_OSYMB],&input[i*NOF_RE_X_OSYMB],NOF_RE_X_OSYMB*sizeof(complex_t));
	}
}


void lte_ch_get_ref_one(complex_t *input, complex_t *output, int data_len) {
	memcpy(output,&input[1],data_len*sizeof(complex_t));
}

void lte_ch_get_ref(complex_t *input, complex_t *output,int nof_prb, int offset, int ref_interval) {
	int i,wp;

	memcpy(output,input,offset*sizeof(complex_t));
	wp=offset;
	for (i=0;i<(NOF_RE_X_OSYMB/ref_interval)*nof_prb-1;i++) {
		lte_ch_get_ref_one(&input[ref_interval*i+offset],&output[wp],ref_interval-1);
		wp+=(ref_interval-1);
	}
	if (ref_interval-offset-1 > 0) {
		lte_ch_get_ref_one(&input[ref_interval*i+offset],&output[wp],ref_interval-offset-1);
	}
}




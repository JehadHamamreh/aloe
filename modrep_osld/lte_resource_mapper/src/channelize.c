#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_lib/grid/base.h"
#include "channelize.h"
#include "channel_setup.h"

extern struct lte_grid_config grid;
extern int subframe_idx;
int nof_channels, nof_pdsch, nof_pdcch, nof_other;

real_t sss_signal[2*SSS_LEN];
complex_t pss_signal[PSS_LEN];
refsignal_t reference_signal;


int read_channels() {
	nof_pdsch=0;
	nof_pdcch=0;
	nof_other=0;
	int max_in_port = -1;

	while(pdsch[nof_pdsch].name) {
		if (pdsch[nof_pdsch].in_port > max_in_port) {
			max_in_port = pdsch[nof_pdsch].in_port;
		}
		nof_pdsch++;
	}
	while(pdcch[nof_pdcch].name) {
		if (pdcch[nof_pdcch].in_port > max_in_port) {
			max_in_port = pdcch[nof_pdcch].in_port;
		}
		nof_pdcch++;
	}
	while(other[nof_other].name) {
		if (other[nof_other].in_port > max_in_port) {
			max_in_port = other[nof_other].in_port;
		}
		nof_other++;
	}
	nof_channels = nof_pdsch+nof_pdcch+nof_other;
	return max_in_port;
}

int check_received_samples_channel(struct channel *ch, int ch_id) {
	int re=0;
	if (ch->in_port>=0) {
		re=lte_ch_get_re(ch_id,ch->type,subframe_idx,&grid);
		if (!get_input_samples(ch->in_port)) {
			return 0;
		}
		moddebug("ch %s port %d rcv_len=%d\n",ch->name,ch->in_port, get_input_samples(ch->in_port));
		if (re != get_input_samples(ch->in_port)) {
			moderror_msg("Received %d samples from channel %s in_port %d, but expected %d "
					"(subframe_idx=%d)\n",
				get_input_samples(ch->in_port),ch->name, ch->in_port,
				re, subframe_idx);
			return -1;
		}
	}
	return re;
}

int check_received_samples_mapper() {
	int i,n,len;
	len=0;

	for (i=0;i<nof_pdsch;i++) {
		n=check_received_samples_channel(&pdsch[i],i);
		if (n<0) {
			return -1;
		}
		len+=n;
	}
	for (i=0;i<nof_pdcch;i++) {
		n=check_received_samples_channel(&pdcch[i],i);
		if (n<0) {
			return -1;
		}
		len+=n;
	}
	for (i=0;i<nof_other;i++) {
		n=check_received_samples_channel(&other[i],0);
		if (n<0) {
			return -1;
		}
		len+=n;
	}
	return len;
}

int init_refsinc_signals() {
#ifdef ENABLE_REF
	reference_signal.port_id = 0;
	generate_cref(&reference_signal,&grid);
#endif
#ifdef ENABLE_PSS
	generate_pss(pss_signal,0,&grid);
#endif
#ifdef ENABLE_SSS
	generate_sss(sss_signal,&grid);
#endif
	return 0;
}

int insert_signals(complex_t *output) {
	int i;
	struct lte_symbol symbol;
	for (i=0;i<grid.nof_osymb_x_subf;i++) {
		symbol.subframe_id = subframe_idx;
		symbol.symbol_id = i;
#ifdef ENABLE_PSS
		lte_pss_put(pss_signal,&output[i*grid.fft_size+grid.pre_guard],&symbol,&grid);
#endif
#ifdef ENABLE_SSS
		if (subframe_idx == 0) {
			lte_sss_put(sss_signal,&output[i*grid.fft_size+grid.pre_guard],&symbol,&grid);
		} else if (subframe_idx == 5) {
			lte_sss_put(&sss_signal[SSS_LEN],&output[i*grid.fft_size+grid.pre_guard],&symbol,&grid);
		}
#endif
#ifdef ENABLE_REF
		lte_refsig_put(&reference_signal,&output[i*grid.fft_size],&symbol,&grid);
#endif
	}
	return 0;
}

int allocate_channel(struct channel *ch, int ch_id, void **inp, void *output) {
	if (!inp[ch->in_port]) {
		return 0;
	}

	return lte_ch_put_sf(inp[ch->in_port],output,ch->type,ch_id,subframe_idx,&grid);
}

int allocate_all_channels(void **inp,void *output) {
	int i;
	if (!output) {
		return -1;
	}
	for (i=0;i<nof_pdsch;i++) {
		if (pdsch[i].in_port >=0) {
			if (allocate_channel(&pdsch[i],i,inp,output) == -1) {
				moderror_msg("Allocating PDSCH channel %d\n",i);
				return -1;
			}
		}
	}
	for (i=0;i<nof_pdcch;i++) {
		if (pdcch[i].in_port >=0) {
			if (allocate_channel(&pdcch[i],i,inp,output) == -1) {
				moderror_msg("Allocating PDCCH channel %d\n",i);
				return -1;
			}
		}
	}
	for (i=0;i<nof_other;i++) {
		if (other[i].in_port >=0) {
			if (allocate_channel(&other[i],0,inp,output) == -1) {
				moderror_msg("Allocating %s channel %d\n",other[i].name,i);
				return -1;
			}
		}
	}
	insert_signals(output);
	lte_set_guard_sf(output,&grid);
	set_output_samples(0,grid.nof_osymb_x_subf*grid.fft_size);
	return 0;
}

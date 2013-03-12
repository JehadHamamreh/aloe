#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_lib/grid/base.h"
#include "channel.h"
#include "channel_setup.h"

extern struct lte_grid_config grid;
extern int direction;
extern int subframe_idx;
int nof_channels, nof_pdsch, nof_pdcch, nof_other;

real_t sss_signal[SSS_LEN];
complex_t pss_signal[PSS_LEN];
refsignal_t reference_signal;


void read_channels(int *max_in_port, int *max_out_port) {
	nof_pdsch=0;
	nof_pdcch=0;
	nof_other=0;
	*max_in_port = -1;
	*max_out_port = -1;

	while(pdsch[nof_pdsch].name) {
		if (pdsch[nof_pdsch].in_port > *max_in_port) {
			*max_in_port = pdsch[nof_pdsch].in_port;
		}
		if (pdsch[nof_pdsch].out_port > *max_out_port) {
			*max_out_port = pdsch[nof_pdsch].out_port;
		}
		nof_pdsch++;
	}
	while(pdcch[nof_pdcch].name) {
		if (pdcch[nof_pdcch].in_port > *max_in_port) {
			*max_in_port = pdcch[nof_pdcch].in_port;
		}
		if (pdcch[nof_pdcch].out_port > *max_out_port) {
			*max_out_port = pdcch[nof_pdcch].out_port;
		}
		nof_pdcch++;
	}
	while(other[nof_other].name) {
		if (other[nof_other].in_port > *max_in_port) {
			*max_in_port = other[nof_other].in_port;
		}
		if (other[nof_other].out_port > *max_out_port) {
			*max_out_port = other[nof_other].out_port;
		}
		nof_other++;
	}
	nof_channels = nof_pdsch+nof_pdcch+nof_other;
}

int check_received_samples_channel(struct channel *ch, int ch_id) {
	int re=0;
	if (ch->in_port>=0) {
		re=lte_ch_get_re(ch_id,ch->type,subframe_idx,&grid);
		if (!get_input_samples(ch->in_port)) {
			return 0;
		}
		if (re != get_input_samples(ch->in_port)) {
			moderror_msg("Received %d samples from channel %s in_port %d, but expected %d "
					"(subframe_idx=%d, direction=%d)\n",
				get_input_samples(ch->in_port),ch->name, ch->in_port,
				re, subframe_idx, direction);
			return -1;
		}
	}
	return re;
}

int check_received_samples_demapper() {
	if (!get_input_samples(0)) {
		return 0;
	}
	if (get_input_samples(0) != grid.fft_size*grid.nof_osymb_x_subf) {
		moderror_msg("Received %d samples from input 0, but expected %d "
				"(subframe_idx=%d, direction=%d)\n",
			get_input_samples(0), grid.fft_size*grid.nof_osymb_x_subf, subframe_idx, direction);
		return -1;
	}
	return get_input_samples(0);
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
	generate_sss(sss_signal,0,&grid);
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
		lte_sss_put(sss_signal,&output[i*grid.fft_size+grid.pre_guard],&symbol,&grid);
#endif
#ifdef ENABLE_REF
		lte_refsig_put(&reference_signal,&output[i*grid.fft_size+grid.pre_guard],&symbol,&grid);
#endif
		}
}

/**
 * TODO: Transform reference signal into a vector for correlation in the next module
 */
int refsig_to_vector(refsignal_t *signal, complex_t *vector) {
	return 2*grid.nof_prb;
}

int extract_refsig(void *in, void **out) {
	int i,n,wp;
	struct lte_symbol symbol;
	complex_t *input = in;

	if (EXTRACT_REF_PORT >= 0) {
		wp=0;
		for (i=0;i<grid.nof_osymb_x_subf;i++) {
			symbol.subframe_id = subframe_idx;
			symbol.symbol_id = i;
			reference_signal.port_id = 0;
			n = lte_refsig_get(&input[i*grid.fft_size+grid.pre_guard],
					&reference_signal,&symbol,&grid);
			if (n<0) {
				return -1;
			}
			wp+=n;
		}
		wp = refsig_to_vector(&reference_signal,out[EXTRACT_REF_PORT]);
		set_output_samples(EXTRACT_REF_PORT,wp);
	}
	return 0;
}

int allocate_channel(struct channel *ch, int ch_id, void **inp, void *output) {
	if (!inp[ch->in_port]) {
		return -1;
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
				return -1;
			}
		}
	}
	for (i=0;i<nof_pdcch;i++) {
		if (pdcch[i].in_port >=0) {
			if (allocate_channel(&pdcch[i],i,inp,output) == -1) {
				return -1;
			}
		}
	}
	for (i=0;i<nof_other;i++) {
		if (other[i].in_port >=0) {
			if (allocate_channel(&other[i],0,inp,output) == -1) {
				return -1;
			}
		}
	}
	insert_signals(output);
	lte_set_guard_sf(output,&grid);
	set_output_samples(0,grid.nof_osymb_x_subf*grid.fft_size);
	return 0;
}

int deallocate_channel(struct channel *ch, int ch_id, void *input, void **out) {
	int n;
	if (!out[ch->out_port]) {
		return -1;
	}

	n = lte_ch_get_sf(input,out[ch->out_port],ch->type,ch_id,subframe_idx,&grid);
	if (n<0) {
		return -1;
	}
	if (n>0) {
		set_output_samples(ch->out_port,n);
	}
	return n;
}

int deallocate_all_channels(void *input, void **out) {
	int i;

	if (!input) {
		return -1;
	}

	for (i=0;i<nof_pdsch;i++) {
		if (pdsch[i].out_port >=0) {
			if (deallocate_channel(&pdsch[i],i,input,out) == -1) {
				return -1;
			}
		}
	}
	for (i=0;i<nof_pdcch;i++) {
		if (pdcch[i].out_port >=0) {
			if (deallocate_channel(&pdcch[i],i,input,out) == -1) {
				return -1;
			}
		}
	}
	for (i=0;i<nof_other;i++) {
		if (other[i].out_port >=0) {
			if (deallocate_channel(&other[i],0,input,out) == -1) {
				return -1;
			}
		}
	}
	return 0;
}

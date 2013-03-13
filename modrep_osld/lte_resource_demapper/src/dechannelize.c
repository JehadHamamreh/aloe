#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_lib/grid/base.h"
#include "dechannelize.h"
#include "channel_setup.h"

extern struct lte_grid_config grid;
extern int subframe_idx;
int nof_channels, nof_pdsch, nof_pdcch, nof_other;

real_t sss_signal[SSS_LEN];
complex_t pss_signal[PSS_LEN];
refsignal_t reference_signal;


int read_channels() {
	nof_pdsch=0;
	nof_pdcch=0;
	nof_other=0;
	int max_out_port = -1;

	while(pdsch[nof_pdsch].name) {
		if (pdsch[nof_pdsch].out_port > max_out_port) {
			max_out_port = pdsch[nof_pdsch].out_port;
		}
		nof_pdsch++;
	}
	while(pdcch[nof_pdcch].name) {
		if (pdcch[nof_pdcch].out_port > max_out_port) {
			max_out_port = pdcch[nof_pdcch].out_port;
		}
		nof_pdcch++;
	}
	while(other[nof_other].name) {
		if (other[nof_other].out_port > max_out_port) {
			max_out_port = other[nof_other].out_port;
		}
		nof_other++;
	}
	if (COPY_SIGNAL_PORT > max_out_port) {
		max_out_port = COPY_SIGNAL_PORT;
	}
	nof_channels = nof_pdsch+nof_pdcch+nof_other;
	return max_out_port;
}

int check_received_samples_demapper() {
	if (!get_input_samples(0)) {
		return 0;
	}
	if (get_input_samples(0) != grid.fft_size*grid.nof_osymb_x_subf) {
		moderror_msg("Received %d samples from input 0, but expected %d "
				"(subframe_idx=%d)\n",
			get_input_samples(0), grid.fft_size*grid.nof_osymb_x_subf, subframe_idx);
		return -1;
	}
	return get_input_samples(0);
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

int copy_signal(void *in, void **out) {
	if (COPY_SIGNAL_PORT != -1) {
		if (out[COPY_SIGNAL_PORT]) {
			memcpy(out[COPY_SIGNAL_PORT],in,get_input_samples(0)*sizeof(complex_t));
			set_output_samples(COPY_SIGNAL_PORT,get_input_samples(0));
		}
	}
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

int is_enabled(int pm_id, int *channel_ids, int nof_channels) {
	int i;
	for (i=0;i<nof_channels;i++) {
		if (channel_ids[i] == pm_id) {
			return 1;
		}
	}
	return 0;
}

int deallocate_all_channels(int *channel_ids, int nof_channels, void *input, void **out) {
	int i;

	if (!input) {
		return -1;
	}

	for (i=0;i<nof_pdsch;i++) {
		if (pdsch[i].out_port >=0) {
			if (is_enabled(pdsch[i].param_value,channel_ids,nof_channels)) {
				if (deallocate_channel(&pdsch[i],i,input,out) == -1) {
					return -1;
				}
			}
		}
	}
	for (i=0;i<nof_pdcch;i++) {
		if (pdcch[i].out_port >=0) {
			if (is_enabled(pdcch[i].param_value,channel_ids,nof_channels)) {
				if (deallocate_channel(&pdcch[i],i,input,out) == -1) {
					return -1;
				}
			}
		}
	}
	for (i=0;i<nof_other;i++) {
		if (other[i].out_port >=0) {
			if (is_enabled(other[i].param_value,channel_ids,nof_channels)) {
				if (deallocate_channel(&other[i],0,input,out) == -1) {
					return -1;
				}
			}
		}
	}
	return 0;
}

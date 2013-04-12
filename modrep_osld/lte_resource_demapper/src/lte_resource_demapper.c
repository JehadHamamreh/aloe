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
#include <string.h>
#include <complex.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_resource_demapper.h"
#include "lte_lib/grid/base.h"
#include "dechannelize.h"

extern int nof_input_itf;
extern int nof_output_itf;

int subframe_idx;
int last_tslotidx;

pmid_t subframe_idx_id;
int nof_channels;

#define MAX_CHANNELS 10

int channel_ids[MAX_CHANNELS];

struct lte_grid_config grid;



/**
 * @ingroup lte_resource_mapper
 *
 *	See lte_ctrl_tx for grid params.
 *
 * \param nof_channels Number of channels to extract
 * \param channel_id_n Id of the n-th channel to extract (configured in channel_setup.h)
 * \param (optional) subframe_idx, if not provided, subframe_idx is counted automatically starting at zero
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int i;
	char tmp[64];
	int max_out_port;

	max_out_port = read_channels();

	grid.fft_size = 128;
	grid.nof_prb = 6;
	grid.nof_osymb_x_subf = 14;
	grid.nof_ports = 1;
	grid.cell_id = 0;
	grid.cfi = -1;
	grid.nof_pdsch = 0;
	grid.nof_pdcch = 0;

	subframe_idx_id = param_id("subframe_idx");
	if (!subframe_idx_id) {
		subframe_idx = 0;
	}

	if (param_get_int_name("nof_channels",&nof_channels)) {
		nof_channels = 1;
	}
	if (nof_channels > MAX_CHANNELS) {
		moderror_msg("Maximum allowed channels is %d (%d)\n",nof_channels,MAX_CHANNELS);
		return -1;
	}
	for (i=0;i<nof_channels;i++) {
		snprintf(tmp,64,"channel_id_%d",i);
		if (param_get_int_name(tmp,&channel_ids[i])) {
			channel_ids[i] = i;
		}
	}

	nof_output_itf = max_out_port+1;
	nof_input_itf = 1;


	return 0;
}

/**
 * @ingroup lte_resource_mapper
 *
 *  Main DSP function
 *
 * This function allocates the data symbols of one slot (0.5 ms) in the corresponding place
 * of the LTE frame grid. During initialization phase CRS, PSS and SSS signals has been
 * incorporated and will no be modified during run phase.
 * PBCH (Physical Broadcast Channel), PCFICH (Physical Control Format Indicator Channel) &
 * PDCCH (Physical Downlink Control Channel) will be incorporated.
 *
 */
int work(void **inp, void **out) {
	int n;
	int cfi;
	subframe_idx=-1;
	if (subframe_idx_id) {
		param_get_int(subframe_idx_id, &subframe_idx);
	}

#ifdef _COMPILE_ALOE
	moddebug("subframe_idx=%d tstamp=%d rcv_len=%d cfi=%d\n",subframe_idx,oesr_tstamp(ctx),
		get_input_samples(0),grid.cfi);
#endif

	n=check_received_samples_demapper();
	if (n < 1) {
		return n;
	}

	if (channels_init_grid(channel_ids, nof_channels)) {
		moderror("Initiating resource grid\n");
		return -1;
	}

	if (deallocate_all_channels(channel_ids, nof_channels, inp[0],out)) {
		return -1;
	}
	if (extract_refsig(inp[0],out)) {
		return -1;
	}

	if (copy_signal(inp[0],out)) {
		return -1;
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


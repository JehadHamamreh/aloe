/* 
 * Copyright (c) 2012
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

#include "lte_resource_mapper.h"
#include "lte_lib/grid/base.h"
#include "channelize.h"

extern int nof_input_itf;
extern int nof_output_itf;

int subframe_idx;
int last_tslotidx;

pmid_t subframe_idx_id;

struct lte_grid_config grid;

#define CHECK_RCV_SAMPLES


/**
 * @ingroup lte_resource_mapper
 *
 *	See lte_ctrl_tx for grid params.
 *
 * \param (optional) subframe_idx, if not provided, subframe_idx is counted automatically starting at zero
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int max_in_port;

	max_in_port = read_channels();

	grid.fft_size = 128;
	grid.nof_prb = 6;
	grid.nof_osymb_x_subf = 14;
	grid.nof_ports = 1;
	grid.cell_id = 0;
	grid.cfi = 1;
	grid.nof_pdcch = 1;
	grid.pdcch[0].nof_cce = 2;
	grid.nof_pdsch = 1;
	grid.pdsch[0].rbg_mask=0xFFFF;
	grid.verbose = 1;

	if (lte_grid_init(&grid)) {
		moderror("Initiating channels grid\n");
		return -1;
	}

	nof_output_itf = 1;
	nof_input_itf = max_in_port+1;

	if (init_refsinc_signals()) {
		return -1;
	}

	subframe_idx_id = param_id("subframe_idx");
	if (!subframe_idx_id) {
		subframe_idx = 0;
	}

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

	if (subframe_idx_id) {
		param_get_int(subframe_idx_id, &subframe_idx);
	}
#ifdef CHECK_RCV_SAMPLES
	int n;
	n=check_received_samples_mapper();
	if (n < 1) {
		return n;
	}
#endif

#ifdef _COMPILE_ALOE
	moddebug("subframe_idx=%d tstamp=%d rcv=%d\n",subframe_idx,oesr_tstamp(ctx),get_input_samples(0));
#endif

	if (allocate_all_channels(inp,out[0])) {
		moderror("Error allocating channels\n");
		return 0;
	}

	subframe_idx++;
	if (subframe_idx==NOF_SUBFRAMES_X_FRAME) {
		subframe_idx=0;
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


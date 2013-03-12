/* 
 * Copyright (c) 2012, Xavier Arteaga, Antoni Gelonch <antoni@tsc.upc.edu> &
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

#include <time.h>

#include "lte_resource_mapper.h"
#include "lte_lib/grid/base.h"
#include "channel.h"

extern int nof_input_itf;
extern int nof_output_itf;

int direction;
int subframe_idx;
int last_tslotidx;

pmid_t subframe_idx_id;

struct lte_grid_config grid;


/**
 * @ingroup lte_resource_mapper
 *
 *	See lte_ctrl_tx for grid params.
 *
 * \param direction 0 for the transmitter side, 1 for the receiver side
 * \param (optional) subframe_idx, if not provided, subframe_idx is counted automatically starting at zero
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int max_in_port,max_out_port;
	struct timeval tdata[3];

	read_channels(&max_in_port,&max_out_port);

	grid.fft_size = 128;
	grid.cfi = 1;
	grid.nof_prb = 6;
	grid.nof_osymb_x_subf = 14;
	grid.nof_ports = 1;
	grid.cell_id = 0;
	grid.nof_pdcch = 0;
	grid.nof_pdsch = 1;
	grid.pdsch[0].rbg_mask = 0xFFFF;

	if (lte_grid_init(&grid)) {
		moderror("Initiating resource grid\n");
		return -1;
	}

	if (param_get_int_name("direction",&direction)) {
		moderror("Parameter direction must be specified\n");
		return -1;
	}

	if (!direction) {
		nof_output_itf = 1;
		nof_input_itf = max_in_port+1;
	} else {
		nof_output_itf = max_out_port+1;
		nof_input_itf = 1;
	}

	if (!direction) {
		if (init_refsinc_signals()) {
			return -1;
		}
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
	int n;


	if (subframe_idx_id) {
		param_get_int(subframe_idx_id, &subframe_idx);
	}

#ifdef _COMPILE_ALOE
	moddebug("subframe_idx=%d tstamp=%d last=%d\n",subframe_idx,oesr_tstamp(ctx),last_tslotidx);
#endif

	if (!direction) {
		n=check_received_samples_mapper();
	} else {
		n=check_received_samples_demapper();
	}
	if (n < 1) {
		return n;
	}

	if (!direction) {
		if (allocate_all_channels(inp,out[0])) {
			return -1;
		}
	} else {
		if (deallocate_all_channels(inp[0],out)) {
			return -1;
		}
		if (extract_refsig(inp[0],out)) {
			return -1;
		}
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


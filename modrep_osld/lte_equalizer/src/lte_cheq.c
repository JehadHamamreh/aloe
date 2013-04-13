/*
 * Copyright (c) 2012,
 * Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * Xavier Arteaga
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
#include <oesr.h>
#include <params.h>
#include <string.h>
#include <skeleton.h>

#include "lte_lib/grid/base.h"

#include "lte_cheq.h"
#include "equalizer.h"


filter2d_t filter;								// Declare Equalizer structure

params_t params [] = {
		{"ntime", 	&filter.ntime, 	DEFAULT_NTIME},
		{"nfreq", 	&filter.nfreq, 	DEFAULT_NFREQ},
		{NULL, NULL, 0}
};


struct lte_grid_config grid;
refsignal_t refsignal;

pmid_t subframe_idx_id;
static int subframe_idx;

/**
 * @ingroup lte_equalizer
 *
 * \param ntime (Optional) 2-D filter time dimension size (Default 14)
 * \param nfreq (Optional)2-D filter freq dimension size (Default 11)
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int i;

	/* Obtain the configuration parameters */
	i=0;
	while (params[i].var != NULL){
		if (param_get_int_name((char*) params[i].name, params[i].var)) {
			*params[i].var = params[i].value;
			modinfo_msg("Parameter %s not defined, setting default %d\n", params[i].name, *params[i].var);
		}
		i++;
	}

	/* set default grid values */
	grid.fft_size = 128;
	grid.nof_prb = 6;
	grid.nof_osymb_x_subf = 14;
	grid.nof_ports = 1;
	grid.cell_id = 0;
	grid.verbose = 0;
	grid.debug = 0;
	refsignal.port_id = 0;

	if (lte_grid_init_params(&grid)) {
		moderror("Initiating grid\n");
		return -1;
	}

	generate_cref(&refsignal,&grid);

	if (filter2d_init (&filter)) {
		moderror("Initiating channel equalizer\n");
		return -1;
	}

	subframe_idx_id = param_id("subframe_idx");
	if (!subframe_idx_id) {
		subframe_idx = 0;
	}

	return 0;
}

int work(void **inp, void **out) {
	int rcv_samples, snd_samples;
	input_t *input;
	output_t *output;

	input = inp[0];
	output = out[0];
	rcv_samples = get_input_samples(0);

	if (!rcv_samples) {
		return 0;
	}

	if (rcv_samples != grid.nof_osymb_x_subf*grid.fft_size) {
		modinfo_msg("WARNING: The number of input samples (%d) is different than expected (%d).\n",
				rcv_samples, grid.nof_osymb_x_subf*grid.fft_size);
		return 0;
	}

	snd_samples = rcv_samples;

	equalizer (&refsignal, subframe_idx,input, output, &filter,&grid);

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


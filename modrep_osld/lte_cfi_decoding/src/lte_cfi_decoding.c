/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
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
#include <skeleton.h>

#include "lte_cfi_decoding.h"
#include "cfi_decoding.h"

char table[4][32];
int cfi_pm_idx;

/**
 * @ingroup lte_cfi_decoding
 * Initializes the CFI coding table for CFI deconding.
 * Physical control format indicator channel (PCFICH)
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	coding_table(table);

	/* get ctrl cfi param index */
#ifdef _COMPILE_ALOE
	cfi_pm_idx = oesr_get_variable_idx(ctx, "ctrl","cfi_rx");
	if (cfi_pm_idx < 0) {
		moderror("Error getting remote parameter cfi_rx\n");
	}
	modinfo_msg("Remote CFI parameter is a at %d\n",cfi_pm_idx)
#endif
	return 0;
}

/**
 * @ingroup lte_cfi_decoding
 * Main DSP function
 * Calls the cfi_decoding() function to decode the control format indicator (CFI)
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 */
int work(void **inp, void **out) {
	int rcv_samples;
	int index;
	input_t *input;

	input = inp[0];
	rcv_samples = get_input_samples(0);

	if (!rcv_samples) {
		return 0;
	}

	if (rcv_samples != NOF_BITS) {
		moderror_msg("Wrong input sequence length (%d). Should be of 32 "
			"bits.\n", rcv_samples);
		return -1;
	}
	index = cfi_decoding(input, table)+1;

#ifdef _COMPILE_ALOE
	moddebug("Detected CFI=%d at tstamp=%d\n",index,oesr_tstamp(ctx));
#endif
	if (param_remote_set(out, 0, cfi_pm_idx, &index, sizeof(int))) {
		moderror("Setting parameter\n");
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

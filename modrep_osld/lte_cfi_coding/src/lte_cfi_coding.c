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
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_cfi_coding.h"
#include "lte_lib/cfi_coding_table.h"

//pmid_t coding_id;
char table[4][32];


/**
 * @ingroup lte_cfi_coding
 * Initializes the CFI coding table for CFI enconding.
 * Physical control format indicator channel (PCFICH)
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	coding_table(table);
	return 0;
}


/**
 * @ingroup lte_cfi_coding
 * Main DSP function
 * Encodes the control format indicator (CFI)
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 */
int work(void **inp, void **out) {
	int i;
	int rcv_samples, snd_samples;
	input_t *input;
	output_t *output;
	int cfi;

	input = inp[0];
	output = out[0];
	rcv_samples = get_input_samples(0);

	if (!rcv_samples) {
		if (param_get_int_name("cfi",&cfi)) {
			moderror("No input nor parameter CFI received\n");
			return -1;
		}
	} else {
		cfi = input[0];
	}

	snd_samples = NOF_BITS;

	if ((cfi > 0) && (cfi < 5)) {
		memcpy(output, &table[cfi-1], NOF_BITS);
	} else {
		moderror_msg("Wrong cfi %d. Specify 1, 2, 3, or 4 (4 reserved)."
		  "\n", input[0]);
		return -1;
	}
	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

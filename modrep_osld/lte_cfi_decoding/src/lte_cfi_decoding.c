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


/**
 * @ingroup lte_cfi_decoding
 * Initializes the CFI coding table for CFI deconding.
 * Physical control format indicator channel (PCFICH)
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	coding_table(table);
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
	int i;
	int rcv_samples, snd_samples;
	int index;
	input_t *input;
	output_t *output;

	input = inp[0];
	output = out[0];
	rcv_samples = get_input_samples(0);
	snd_samples = 1;

	if (rcv_samples != NOF_BITS) {
		moderror_msg("Wrong input sequence length (%d). Should be of 32 "
			"bits.\n", rcv_samples);
		return -1;
	}
	index = cfi_decoding(input, table);
	output[0] = index+1;
/*	printf("\n");
	for (i=0; i<snd_samples; i++) {
		printf("%d",output[i]);
	}
*/
	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

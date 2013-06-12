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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include <skeleton.h>
#include <params.h>

#define INCLUDE_DEFS_ONLY
#include "gen_conv.h"


/**
 *  Generates input signal. VERY IMPORTANT to fill length vector with the number of
 * samples that have been generated.
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 */
int generate_input_signal(void *in, int *lengths)
{
	int i;
	input_t *input = in;
	input_t *filter = &input[INPUT_MAX_SAMPLES];
	int input_len,filter_len;

	if (param_get_int_name("input_len",&input_len)) {
		moderror("Getting integer parameter input_len\n");
		return -1;
	}
	if (param_get_int_name("filter_len",&filter_len)) {
		moderror("Getting integer parameter filter_len\n");
		return -1;
	}


	/** HERE INDICATE THE LENGTH OF THE SIGNAL */
	lengths[0] = input_len;
	lengths[1] = filter_len;

	for (i=0;i<input_len;i++) {
		__real__ input[i] = (float) ((i)%(input_len));
		__imag__ input[i] = (float) ((input_len-i-1)%(input_len));
	}
	for (i=0;i<filter_len;i++) {
		__real__ filter[i] = (float) ((i)%(filter_len));
		__imag__ filter[i] = (float) ((filter_len-i-1)%(filter_len));
	}
	return 0;
}

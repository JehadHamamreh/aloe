/* 
 * Copyright (c) 2012, Vuk Marojevic <marojevic@tsc.upc.edu>.
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
#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "lte_ctrl_ratematching.h"

/**
 * Generates input signal. VERY IMPORTANT to fill length vector with the number of
 * samples that have been generated.
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 */
int generate_input_signal(void *in, int *lengths)
{
	int i;
	char *input_b;
	float *input_f;
	char test_in_b[8];
	float test_in_f[8];

	int block_length;
	pmid_t blen_id, direction_id;
	int direction;

	blen_id = param_id("block_length");
	if (!blen_id) {
		moderror("Parameter block_length not found\n");
		return -1;
	}
	if (!param_get_int(blen_id,&block_length)) {
		moderror("Getting integer parameter block_length\n");
		return -1;
	}

	direction_id = param_id("direction");
	if (!direction_id) {
		moderror("Parameter direction not found. Assuming transmission mode.\n");
		direction = 0;
	} else {
		if (!param_get_int(direction_id, &direction)) {
			moderror("Getting integer parameter direction. Assuming transmission mode.\n");
			direction = 0;
		}
	}

	modinfo_msg("Parameter block_length is %d\n",block_length);
	modinfo_msg("Direction is %d (0: transmission, 1: reception mode).\n",direction);

	/** HERE INDICATE THE LENGTH OF THE SIGNAL */
	lengths[0] = block_length;

	if (!direction) {
		input_b = in;
		test_in_b[0] = 0x0;
		test_in_b[1] = 0x0;
		test_in_b[2] = 0x1;
		test_in_b[3] = 0x0;
		test_in_b[4] = 0x1;
		test_in_b[5] = 0x1;
		test_in_b[6] = 0x1;
		test_in_b[7] = 0x0;
		for (i=0; i<block_length; i++) {
			input_b[i] = test_in_b[i%8];
		}
	} else {
		input_f = in;
		test_in_f[0] = -0.1;
		test_in_f[1] = 0.2;
		test_in_f[2] = -0.3;
		test_in_f[3] = 0.4;
		test_in_f[4] = -0.5;
		test_in_f[5] = 0.6;
		test_in_f[6] = -0.7;
		test_in_f[7] = 0.8;
		for (i=0; i<block_length; i++) {
			input_f[i] = test_in_f[i%8];
		}
	}

	return 0;
}

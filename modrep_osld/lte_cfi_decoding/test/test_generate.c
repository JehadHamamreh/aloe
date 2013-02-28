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
#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "lte_cfi_decoding.h"
#include "cfi_decoding.h"

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
	input_t *input = in;

	/** LENGTH OF THE INPUT SIGNAL */
	lengths[0] = NOF_BITS;

	for (i=0;i<NOF_BITS;i++) {
		input[i] = 0x1;
	}
	input[1] = 0x0;
	input[31] = 0x0;
	return 0;
}

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

#include "gen_mux.h"

#include "base/types.h"
#include "base/vector.h"
#include "utils/mux.h"

int data_type;
int nof_inputs;

extern int input_sample_sz;
extern int output_sample_sz;
extern int nof_input_itf;

int input_lengths[NOF_INPUT_ITF];
int input_padding_pre[NOF_INPUT_ITF];

/** @ingroup gen_mux gen_mux
 *
 *	\param data_type 0 for bits, 1 for real samples, 2 for complex samples
 *	\param nof_inputs Number of connected input interfaces
 */

int initialize() {
	sample_t d_type;

	if (param_get_int_name("data_type",&data_type)) {
		moderror("Error getting parameter data_type\n");
		return -1;
	}

	if (type_param_2_type(data_type,&d_type)) {
		moderror_msg("Invalid data_type parameter %d\n",data_type);
		return -1;
	}
	input_sample_sz = type_size(d_type);
	output_sample_sz = input_sample_sz;

	if (param_get_int_name("nof_inputs",&nof_inputs)) {
		moderror("Error getting parameter nof_inputs\n");
		return -1;
	}
	nof_input_itf = nof_inputs;
	if (nof_inputs > NOF_INPUT_ITF) {
		moderror_msg("Only %d interfaces are supported. nof_inputs=%d\n",NOF_INPUT_ITF,nof_inputs);
		return -1;
	}
	memset(input_padding_pre,0,sizeof(int)*nof_inputs);
	return 0;
}

/**@ingroup gen_mux
 *
 */
int work(void **inp, void **out) {
	int i;
	int out_len;

	for (i=0;i<nof_inputs;i++) {
		input_lengths[i] = get_input_samples(i);
	}
	out_len = sum_i(input_lengths,nof_inputs);
	if (out_len) {
		mux(inp,out[0],input_lengths,input_padding_pre,nof_inputs,input_sample_sz);
	}
	return out_len;
}

int stop() {
	return 0;
}


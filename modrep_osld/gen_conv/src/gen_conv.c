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
#include <string.h>

#include "dft/dft.h"
#include "base/types.h"
#include "base/conv.h"
#include "gen_conv.h"

int conv_fft=0,filter_len,input_len;

struct conv_fft_cc fft_state;

int initialize() {

	param_get_int_name("conv_fft",&conv_fft);
	filter_len=0;
	param_get_int_name("filter_len",&filter_len);

	input_len=0;
	param_get_int_name("input_len",&input_len);

	if (conv_fft && (!filter_len || !input_len)) {
		moderror("Parameter filter_len and input_len must be greater than 0\n");
		return -1;
	}

	if (conv_fft) {
		if (conv_fft_cc_init(&fft_state, input_len, filter_len)) {
			moderror("Creating fft plan\n");
			return -1;
		}
	}

	return 0;
}


int work(void **inp, void **out) {
	int input_len,filter_len,output_len;
	input_t *input,*filter;
	output_t *output;

	input_len=get_input_samples(0);
	filter_len=get_input_samples(1);

	if (!input_len) {
		return 0;
	}

	input=inp[0];
	filter=inp[1];
	output=out[0];

	if (conv_fft) {
		output_len = conv_fft_cc_run(&fft_state, input, filter, output);
	} else {
		output_len = conv_cc(input, filter, output, input_len, filter_len);
	}

	return output_len;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


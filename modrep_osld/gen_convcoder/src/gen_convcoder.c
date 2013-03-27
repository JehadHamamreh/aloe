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

#include "gen_convcoder.h"
#include "conv_encode.h"

pmid_t padding_id;
static int padding;

int tail_bit;
int constraint_length;
int rate;
int g[MAX_RATE];


/** @ingroup gen_convcoderr
 * \param padding (Optional) Default 0. Number of zero bits to add to the output after encoding the data.
 * \param constraint_length
 * \param tail_bit 1 for tail-bitting, 0 for none
 * \param rate
 * \param generator_i polynomy of the i-th generator
 */
int initialize() {
	int i;
	char tmp[64];

	padding_id = param_id("padding");
	if (!padding_id) {
		modinfo("Parameter padding not configured. Setting to 0\n");
		padding = 0;
	}
	if (param_get_int_name("constraint_length",&constraint_length)) {
		moderror("Parameter constraint_length not specified\n");
		return -1;
	}
	if (param_get_int_name("tail_bit",&tail_bit)) {
		moderror("Parameter tail_bit not specified\n");
		return -1;
	}
	if (param_get_int_name("rate",&rate)) {
		moderror("Parameter rate not specified\n");
		return -1;
	}
	if (rate<0 || rate > MAX_RATE) {
		moderror_msg("Invalid rate %d\n",rate);
		return -1;
	}
	for (i=0;i<rate;i++) {
		snprintf(tmp,64,"generator_%d",i);
		if (param_get_int_name(tmp,&g[i])) {
			moderror_msg("Parameter %s not specified\n",tmp);
			return -1;
		}
	}

	return 0;
}


int work(void **inp, void **out) {
	int i, out_len, j;
	input_t *input;
	output_t *output;
	int rcv_samples;

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input = inp[i];
		output = out[i];
		moddebug("rcv_len=%d\n",get_input_samples(i));
		rcv_samples = get_input_samples(i);
		if (rcv_samples) {
			conv_encode(input,rcv_samples,constraint_length,rate,g,tail_bit,
								output,&out_len);

			for (j=0;j<padding;j++) {
				output[out_len+j] = 0;
			}

			set_output_samples(i,out_len+padding);
		}
	}
	return 0;
}

int stop() {
	return 0;
}


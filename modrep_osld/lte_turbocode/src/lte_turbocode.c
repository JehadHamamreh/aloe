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

#include "lte_turbocode.h"
#include "turbocoder.h"
#include "turbodecoder.h"

struct turbodecoderConf ccfg;

pmid_t padding_id;
static int padding;

int direction;

extern int input_sample_sz;
extern int output_sample_sz;

/** @ingroup lte_turbocoder
 * \param padding (Optional) Default 0. Number of zero bits to add to the output after encoding the data.
 * \param direction 0 for the encoder, 1 for the decoder
 */
int initialize() {

	padding_id = param_id("padding");
	if (!padding_id) {
		modinfo("Parameter padding not configured. Setting to 0\n");
		padding = 0;
	}
	if (param_get_int_name("direction",&direction)) {
		moderror("Parameter direction not specified\n");
		return -1;
	}
	if (!direction) {
		input_sample_sz = sizeof(char);
		output_sample_sz = sizeof(char);
	} else {
		input_sample_sz = sizeof(float);
		output_sample_sz = sizeof(char);
	}

	return 0;
}


int work(void **inp, void **out) {
	int i, out_len, j;
	input_t *input;
	output_t *output;
	Tdec *input_llr;

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input = inp[i];
		output = out[i];
		moddebug("rcv_len=%d\n",get_input_samples(i));

		if (get_input_samples(i)) {
			if (!direction) {
				out_len = RATE*get_input_samples(i)+TOTALTAIL;
				if (turbo_coder(input,output,get_input_samples(i))<0) {
					return -1;
				}
			} else {
				out_len = (get_input_samples(i)-TOTALTAIL)/RATE;
				input_llr = (Tdec*) input;

				ccfg.Long_CodeBlock=out_len;
				ccfg.Turbo_Dt=100000;
				ccfg.Turbo_iteracions=1;
				ccfg.haltMethod=HALT_METHOD_NONE;

				if (turbo_decoder(input_llr,output,&ccfg, NULL)<0) {
					return -1;
				}
			}
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


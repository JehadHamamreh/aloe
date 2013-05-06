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

#include "lte_ratematching.h"
#include "ratematching.h"
#include "lte_lib/grid/base.h"

pmid_t out_len_id,rvidx_id;

int direction;
extern int input_sample_sz;
extern int output_sample_sz;

/**
 * @ingroup lte_ratematching
 *  Rate adapter for LTE DL/UL. Code rate is out_len/in_len
 *
 * \param out_len Output length
 * \param rv_idx Document this parameter
 * \param direction 0 for the transmitter, 1 for the receiver
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {


	out_len_id = param_id("out_len");
	rvidx_id = param_id("rvidx");

	if (param_get_int_name("direction",&direction)) {
		moderror("Error getting parameter direction\n");
		return -1;
	}

	if (direction) {
		input_sample_sz=sizeof(float);
		output_sample_sz=sizeof(float);
	} else {
		input_sample_sz=sizeof(char);
		output_sample_sz=sizeof(char);
	}

	return 0;
}

/**
 * @ingroup lte_ratematching
 *
 * Main DSP function
 *
 */
int work(void **inp, void **out) {
	int in_len,nof_active_itf,out_len,out_len_block,rvidx;
	int i;
	char *input_b;
	char *output_b;
	float *input_f;
	float *output_f;

	if (!get_input_samples(0)) {
		return 0;
	}

	if (param_get_int(out_len_id, &out_len) != 1) {
		moderror("Error getting parameter out_len\n");
		return -1;
	}

	if (rvidx_id) {
		if (param_get_int(rvidx_id, &rvidx) != 1) {
			moderror("Error getting parameter rvidx\n");
			return -1;
		}
	} else {
		rvidx = 0;
	}

	nof_active_itf = 0;
	for (i=0;i<NOF_INPUT_ITF;i++) {
		if (get_input_samples(i))
			nof_active_itf++;
	}
	
	if (!nof_active_itf) {
		return 0;
	}
	
	out_len_block = out_len/nof_active_itf;
	output_f = out[0];
	output_b = out[0];

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input_f = inp[i];
		input_b = inp[i];
		in_len = get_input_samples(i);

		if (in_len && inp[i] && out[i]) {
			if(!direction) {
				if (char_RM_block(input_b,&output_b[i*out_len_block],in_len,
						out_len_block,rvidx)) {
					return -1;
				}
			} else {
				if (float_UNRM_block(input_f,&output_f[i*out_len_block],in_len,
						out_len_block,rvidx)) {
					return -1;
				}	
			}

		}
	}
	return out_len_block*nof_active_itf;
}

int stop() {
	return 0;
}


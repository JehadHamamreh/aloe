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

#include "gen_demux.h"

#include "base/types.h"
#include "utils/mux.h"
#include "base/vector.h"

int data_type;
int nof_outputs;

extern int input_sample_sz;
extern int output_sample_sz;
extern int nof_output_itf;

int output_lengths[NOF_OUTPUT_ITF];
int output_padding_pre[NOF_OUTPUT_ITF];
int output_padding_post[NOF_OUTPUT_ITF];
int special_output_length[NOF_OUTPUT_ITF];
int sum_special_output_lengths;
int nof_special_outputs;

/** @ingroup gen_demux
 *
 *	\param data_type 0 for bits, 1 for real samples, 2 for complex samples
 *	\param nof_outputs Number of connected output interfaces
 *	\param out_len_i Sets to interface i a different length.
 */

int initialize() {
	int i;
	sample_t d_type;
	char pname[64];

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

	if (param_get_int_name("nof_outputs",&nof_outputs)) {
		moderror("Error getting parameter nof_outputs\n");
		return -1;
	}
	if (nof_outputs > NOF_OUTPUT_ITF) {
		moderror_msg("Only %d interfaces are supported. nof_input_itf=%d\n",NOF_OUTPUT_ITF,nof_outputs);
		return -1;
	}
	memset(special_output_length,0,sizeof(int)*nof_outputs);
	nof_special_outputs = 0;
	for (i=0;i<nof_outputs;i++) {
		snprintf(pname,64,"out_len_%d",i);
		if (!param_get_int_name(pname,&special_output_length[i])) {
			nof_special_outputs++;
		}
	}
	sum_special_output_lengths = sum_i(special_output_length,nof_outputs);
	nof_output_itf = nof_outputs;
	memset(output_padding_pre,0,sizeof(int)*nof_outputs);
	memset(output_padding_post,0,sizeof(int)*nof_outputs);

	return 0;
}

/**@ingroup gen_demux
 *
 */
int work(void **inp, void **out) {
	int i;
	int rcv_len, out_len;

	rcv_len=get_input_samples(0);
	if (!rcv_len)
		return 0;

	if ((rcv_len - sum_special_output_lengths) % (nof_outputs-nof_special_outputs)) {
/*		moderror_msg("Received length %d is not multiple of the number of output interfaces %d\n",
				rcv_len,nof_outputs);
*/		return 0;
	}

	out_len = (rcv_len - sum_special_output_lengths) / (nof_outputs-nof_special_outputs);
	for (i=0;i<nof_outputs;i++) {
		if (special_output_length[i]) {
			output_lengths[i] = special_output_length[i];
		} else {
			output_lengths[i] = out_len;
		}
		set_output_samples(i,output_lengths[i]);
	}

	demux(inp[0],out,output_lengths,output_padding_pre,
			output_padding_post,nof_outputs,input_sample_sz);

	return 0;
}

int stop() {
	return 0;
}


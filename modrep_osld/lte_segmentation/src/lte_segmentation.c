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
#include <math.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>
#include "lte_segmentation_func.h"
#include "lte_segmentation.h"

extern int Table5133[189];

/**
 * @ingroup lte_segmentation
 * This module has no initialization parameters
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	/** Generate Table 5.1.3-3 of ETSI TS 136212 v10.6.0*/
	modinfo("Generating Table 5133\n");
	calculateTable5133(Table5133);

	return 0;
}

/**
 * @ingroup lte_segmentation
 *
 *
 */
int work(void **inp, void **out) {
	int rcv_samples,snd_samples;
	int j, i;
	input_t *input;
	output_t *output;

	int nof_cb, nof_short_cb, nof_long_cb,nof_filler_bits;
	int len_short_cb,len_long_cb;
	int in_idx;

	input = inp[0];
	output = out[0];

	rcv_samples = get_input_samples(0);

	nof_cb=get_nof_cb(rcv_samples, &nof_short_cb, &nof_long_cb,\
			&len_short_cb, &len_long_cb, &nof_filler_bits);

	/** Generate Output*/
	in_idx=0;
	input = inp[0];

	for(i=0; i<nof_cb;i++){
		output = out[i];

		if (!out[i]) {
			moderror_msg("Output interface %d not ready\n",i);
			return -1;
		}

		if(i < nof_short_cb){
			snd_samples = len_short_cb;
		} else {
			snd_samples = len_long_cb;
		}

		if(nof_filler_bits > 0){
			memset(output, 0, sizeof(input_t)*nof_filler_bits);
		}

		memcpy(&output[nof_filler_bits],&input[in_idx],
				sizeof(input_t)*(snd_samples-nof_filler_bits));

		set_output_samples(i,snd_samples);
		
		in_idx += snd_samples-nof_filler_bits;

		nof_filler_bits=0;
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


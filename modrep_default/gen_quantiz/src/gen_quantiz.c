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
#include <math.h>
#include <string.h>

#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "gen_quantiz.h"

pmid_t scale_id,amplitude_id;

/**
 * @ingroup gen_quantiz
 *
 * \param scale
 * \param amplitude
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int size;

	scale_id = param_id("scale");
	if (!scale_id) {
		moderror("Expected parameter scale\n");
		return -1;
	}
	amplitude_id = param_id("amplitude");
	if (!amplitude_id) {
		moderror("Expected parameter amplitude\n");
		return -1;
	}
	return 0;
}

int work(void **inp, void **out) {
	int rcv_samples;
	int i,n;
	input_t *input;
	output_t *output;
	float scale;
	int amplitude;

	if (param_get_float(scale_id, &scale) != 1) {
		moderror("Error getting parameter scale\n");
		return -1;
	}
	if (param_get_int(amplitude_id, &amplitude) != 1) {
		moderror("Error getting parameter amplitude\n");
		return -1;
	}

	for (n=0;n<NOF_INPUT_ITF;n++) {
		input = inp[n];
		output = out[n];
		rcv_samples = get_input_samples(n);
		for (i=0;i<rcv_samples;i++) {
			input[i] /= scale;
			if (__real__ input[i] > 1.0) {
				modinfo_msg("Saturating real %g\n",__real__ input[i]);
				__real__ input[i] = 1.0;
			}
			if (__imag__ input[i] > 1.0) {
				modinfo_msg("Saturating imag %g\n",__imag__ input[i]);
				__imag__ input[i] = 1.0;
			}
			if (__real__ input[i] < -1.0) {
				modinfo_msg("Saturating real %g\n",__real__ input[i]);
				__real__ input[i] = -1.0;
			}
			if (__imag__ input[i] < -1.0) {
				modinfo_msg("Saturating imag %g\n",__imag__ input[i]);
				__imag__ input[i] = -1.0;
			}
			output[i] = (output_t) amplitude*input[i];
		}
		set_output_samples(n,rcv_samples);
	}
	return rcv_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


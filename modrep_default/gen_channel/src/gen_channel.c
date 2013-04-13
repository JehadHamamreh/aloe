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

#include "gen_channel.h"
#include "base/vector.h"
#include "channel.h"

pmid_t scale_id,gain_re_id,gain_im_id,variance_id;

input_t noise_vec[INPUT_MAX_SAMPLES];

float snr_min,snr_max,snr_step, snr_current;
int auto_mode,num_realizations,cnt_realizations;

float get_variance(float snr_db,float scale) {
	return sqrt(pow(10,-snr_db/10)*scale);
}

/**
 * @ingroup gen_channel
 *
 * \param variance Gaussian noise variance
 * \param gain Channel gain
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int size;

	gain_re_id = param_id("gain_re");
	gain_im_id = param_id("gain_im");
	variance_id = param_id("variance");
	scale_id = param_id("noise_scale");

	if (!param_get_float_name("snr_min",&snr_min)
			&& !param_get_float_name("snr_max",&snr_max)
			&& !param_get_float_name("snr_step",&snr_step)
			&& !param_get_int_name("num_realizations",&num_realizations)) {
		auto_mode=1;
		snr_current=snr_min;
		cnt_realizations=0;
	} else {
		auto_mode=0;
	}

	return 0;
}

int work(void **inp, void **out) {
	int rcv_samples;
	int n;
	input_t *input;
	output_t *output;
	float gain_re,gain_im,variance,scale;
	_Complex float gain_c;

	if (!get_input_samples(0)) {
		return 0;
	}

	if (param_get_float(gain_re_id, &gain_re) != 1) {
		moderror("Error getting parameter gain_re\n");
		return -1;
	}
	if (param_get_float(gain_im_id, &gain_im) != 1) {
		moderror("Error getting parameter gain_im\n");
		return -1;
	}
	if (param_get_float(scale_id, &scale) != 1) {
		moderror("Error getting parameter scale\n");
		return -1;
	}
	if (!auto_mode) {
		if (param_get_float(variance_id, &variance) != 1) {
			moderror("Error getting parameter variance\n");
			return -1;
		}
	} else if (auto_mode == 1) {
		variance = get_variance(snr_current,scale);
		cnt_realizations++;
		if (cnt_realizations >= num_realizations) {
			snr_current += snr_step;
			cnt_realizations=0;
		}
		if (snr_current >= snr_max) {
			auto_mode=2;
			variance=0;
		}
	}

	__real__ gain_c = gain_re;
	__imag__ gain_c = gain_im;

	for (n=0;n<NOF_INPUT_ITF;n++) {
		input = inp[n];
		output = out[n];
		rcv_samples = get_input_samples(0);
		if (variance != 0) {
			gen_noise_c(noise_vec,variance,rcv_samples);
			vec_sum_c(output,input,noise_vec,rcv_samples);
		} else {
			memcpy(output,input,rcv_samples*sizeof(input_t));
		}
		if (gain_re != 1.0 && gain_im != 0.0) {
			vec_mult_c(output,gain_c,rcv_samples);
		}

	}
	return rcv_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


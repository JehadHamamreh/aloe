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

#include "gen_agc.h"

#include "base/types.h"
#include "base/vector.h"

pmid_t power_id,scale_id;

real_t signal_abs[INPUT_MAX_SAMPLES];

/**
 * @ingroup gen_agc
 *
 * \param power Output signal power
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	power_id = param_id("power");
	scale_id = param_id("scale");
	return 0;
}

int work(void **inp, void **out) {
	int rcv_samples;
	input_t *input,*output;
	float output_power, input_power, scale;
	
	output_power=1.0;
	param_get_float(power_id, &output_power);
	scale=1.0;
	param_get_float(scale_id, &scale);

	rcv_samples = get_input_samples(0);
	if (!rcv_samples || !out[0]) {
		return 0;
	}

	input=inp[0];
	output=out[0];

	if (!scale_id) {	
		vec_abs(input,signal_abs,rcv_samples);
		input_power = sum_r(signal_abs,rcv_samples);
		if (input_power>0.001) {
			scale = output_power/input_power;
		} else {
			scale = 1.0;
		}
	} else {
		input_power = 0.0;
	}
	vec_mult_c_r(input,output,scale,rcv_samples);
	modinfo_msg("in_power=%g scale=%g\n",input_power,scale);
	return rcv_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


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
#include <rtdal.h>
#include <params.h>
#include <skeleton.h>
#include <complex.h>
#include "dac_sink.h"

extern int input_sample_sz;

static int data_type;
static void *buffer;
pmid_t freq_id, gain_id;
static float last_freq=0;
static int last_rcv_samples = 0;
float max=-9999;
float min=9999;

/**
 * @ingroup dac_sink
 *
 * \param freq_samp Sets DA converter sampling frequency
 */
int initialize() {
	buffer = rtdal_uhd_buffer(1);
	freq_id = param_id("freq_samp");
	if (!freq_id) {
		moderror("Parameter freq_samp not found\n");
		return -1;
	}
	gain_id = param_id("gain");

	if (param_get_int_name("data_type", &data_type)) {
		data_type = 1;
	}
	switch(data_type) {
	case 0:
		input_sample_sz = sizeof(float);
		break;
	case 1:
		input_sample_sz = sizeof(_Complex float);
		break;
	case 2:
		input_sample_sz = sizeof(_Complex short);
		break;
	}

	return 0;
}

/**
 * @ingroup dac_sink
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	int rcv_samples;
	float *input_rf;
	_Complex float *input_f;
	_Complex short *input_s;
	int i,j;
	float freq;
	float gain;
	float x=0;
	float *buffer_rf = buffer;
	_Complex float *buffer_f = buffer;
	_Complex short *buffer_s = buffer;

	if (param_get_float(freq_id,&freq) != 1) {
		moderror("Getting parameter freq_samp\n");
		return -1;
	}

	if (gain_id) {
		if (param_get_float(gain_id,&gain) != 1) {
			moderror("Getting parameter gain\n");
			return -1;
		}
	} else {
		gain = 1.0;
	}

#ifdef _COMPILE_ALOE
	if (freq != last_freq) {
		modinfo_msg("Set sampling frequency to %.2f MHz at tslot %d\n", freq/1000000,oesr_tstamp(ctx));
		last_freq = freq;
	}
#endif

	rtdal_uhd_set_freq(freq);

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input_s = inp[i];
		input_f = inp[i];
		input_rf = inp[i];

		rcv_samples = get_input_samples(i);

#ifdef _COMPILE_ALOE
		if (rcv_samples != last_rcv_samples) {
			last_rcv_samples = rcv_samples;
			modinfo_msg("Receiving %d samples at tslot %d\n",rcv_samples,oesr_tstamp(ctx));
		}
#endif

		rtdal_uhd_set_block_len(rcv_samples);
		x=0;
		for (j=0;j<rcv_samples;j++) {
			switch(data_type) {
			case 0:
				buffer_rf[j] = gain*input_rf[j];
				break;
			case 1:
				buffer_f[j] = gain*input_f[j];
				break;
			case 2:
				buffer_s[j] = gain*input_s[j];
				break;
			}
		}
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


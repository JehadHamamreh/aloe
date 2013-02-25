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

static int sample_is_short;
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

	sample_is_short=0;
	param_get_int_name("sample_is_short",&sample_is_short);

	if (sample_is_short) {
		input_sample_sz = sizeof(_Complex short);
	} else {
		input_sample_sz = sizeof(_Complex float);
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
	_Complex float *input_f;
	_Complex short *input_s;
	int i,j;
	float freq;
	float gain;
	float x=0;
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
			if (sample_is_short) {
				buffer_s[j] = gain*input_s[j];
			} else {
				buffer_f[j] = gain*input_f[j];
			}
		}

#ifdef _COMPILE_ALOE
		if (!x) {
			printf("zeros a ts=%d - %d\n",oesr_tstamp(ctx),rcv_samples);
		}
#endif
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


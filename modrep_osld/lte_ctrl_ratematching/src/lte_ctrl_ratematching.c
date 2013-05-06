/* 
 * Copyright (c) 2012, Vuk Marojevic <marojevic@tsc.upc.edu>.
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
 * along with ALOE++. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_ctrl_ratematching.h"
#define INCLUDE_FUNCTIONS
#include "ctrl_ratematching.h"

pmid_t E_id, S_id;
int direction;

extern int input_sample_sz;
extern int output_sample_sz;


/**
 * @ingroup Lte Rate Matching of convolutionally encoded information
 * Applied for PDCCH
 * Initialization function
 *
 * \param direction Transmitter mode if 0 and receiver mode otherwise
 * \param E Number of output bits of rate matching in transmitter mode
 * \param S Number of output bits of rate matching in receiver mode
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	if (param_get_int_name("direction", &direction)) {
		moderror_msg("Parameter 'direction' not specified. Assuming "
		  "transmission mode (%d).\n", 0);
		direction = 0;
	}
	if (direction) {
		/* Reception mode */
		S_id = param_id("S");
		if (!S_id) {
			moderror("Parameter S not found\n");
			return -1;
		}

		input_sample_sz=sizeof(float);
		output_sample_sz=sizeof(float);
	} else {
		/* Transmission mode */
		E_id = param_id("E");
		if (!E_id) {
			moderror("Parameter E not found\n");
			return -1;
		}
		input_sample_sz=sizeof(char);
		output_sample_sz=sizeof(char);
	}
	return 0;
}


/**
 * @ingroup Lte Rate Matching of convolutionally encoded information
 * Applied for PDCCH
 *
 * Main DSP function
 *
 *
 * \param inp Input interface buffers. The value inp[i] points to the buffer received
 * from the i-th interface. The function get_input_samples(i) returns the number of received
 * samples (the sample size is by default sizeof(input_t))
 *
 * \param out Output interface buffers. The value out[i] points to the buffer where the samples
 * to be sent through the i-th interfaces must be stored.
 *
 * \param E Number of output bits of rate matching in transmitter mode
 * \param S Number of output bits of rate matching in receiver mode
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 */
int work(void **inp, void **out) {
	int S, E;
	int rcv_samples, snd_samples;
	char *input_b;
	char *output_b;
	float *input_f;
	float *output_f;

	rcv_samples = get_input_samples(0);
	if (!rcv_samples || !out[0] || rcv_samples > 3*6114) {
		return 0;
	}

	input_f = inp[0];
	input_b = inp[0];
	output_f = out[0];
	output_b = out[0];

	if (!direction) {
		/* @Transmitter */
		if (param_get_int(E_id, &E) != 1) {
			moderror("Error getting parameter 'E', indiciating the "
			  "number of rate matching output samples in Tx mode.\n");
			return -1;
		}
		if (E > OUTPUT_MAX_SAMPLES) {
			moderror("Too may output samples (E).\n");
			return -1;
		}
		rate_matching(input_b, output_b, rcv_samples, E);
		snd_samples = E;

	} else {
		/* @Receiver */
		if (param_get_int(S_id, &S) != 1) {
			moderror("Error getting parameter 'S', indiciating the "
			  "number of rate matching output samples in Rx mode\n");
			return -1;
		}
		if (S > OUTPUT_MAX_SAMPLES) {
			moderror("Too may output samples (S).\n");
			return -1;
		}
		if (S%3 > 0) {
			moderror("Number of output samples S not integer divisible by 3.\n");
			return -1;
		}
		snd_samples = rate_unmatching(input_f, output_f, rcv_samples, S);
	}

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

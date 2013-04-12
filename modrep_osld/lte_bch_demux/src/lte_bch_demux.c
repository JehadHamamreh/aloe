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
#include <stdlib.h>
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_bch_demux.h"


/*#define ten_chars 10*sizeof(char)*/
#define FRAME_PERIOD 4
char outi[FRAME_PERIOD][OUTPUT_MAX_SAMPLES];
/*#define forT 40*/


/*input_t outi[FRAME_PERIOD*OUTPUT_MAX_SAMPLES];*/

/**
 * @ingroup template
 * Initializes the scrambling sequence based on the parameters:
 * - Codeword index 'q' (0, 1). In case of single codeword: q=0
 * - Cell ID group index 'cell_gr' (0, 1, 2)
 * - Cell ID sector index 'cell_sec' within the physical-layer cell-identity
 *   group (0, 1, ..., 167)
 * - Radio network temporary identifier 'nrnti' (integer [0, 2e16-1=65535])
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	return 0;
}


/**
 * @ingroup LTE BCH Time Demultiplexer
 *
 * Main DSP function
 *
 * Splits the input stream of 40 bits into 4x10 bits, processed in 4 consecutives radio frames
 *
 * \param inp Input interface buffers. The value inp[i] points to the buffer received
 * from the i-th interface. The function get_input_samples(i) returns the number of received
 * samples (the sample size is by default sizeof(input_t))
 *
 * \param out Output interface buffers. The value out[i] points to the buffer where the samples
 * to be sent through the i-th interfaces must be stored.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 */
int work(void **inp, void **out) {
	int i, j, s;
	int rcv_samples, snd_samples;
	static int subframe = 0;
	static int frame = 0;
	static int empty = 1;
	static int samples_per_period;
	input_t *input;
	output_t *output;

	rcv_samples = get_input_samples(0);
	snd_samples = 0;
	if ((!rcv_samples) && (empty)) {
		return 0;
	}
	input = inp[0];
	output = out[0];

	if (rcv_samples) {

		samples_per_period = rcv_samples/FRAME_PERIOD;

		/* Verify parameters */
		if (rcv_samples > INPUT_MAX_SAMPLES) {
			moderror_msg("Too many input samples %d. Maximum is "
			"%d.\n", rcv_samples, INPUT_MAX_SAMPLES);
			return -1;
		}

		if (samples_per_period > OUTPUT_MAX_SAMPLES) {
			moderror_msg("Too many output samples %d. Maximum is "
			"%d.\n", samples_per_period, OUTPUT_MAX_SAMPLES);
			return -1;
		}
		if ((samples_per_period*FRAME_PERIOD) < rcv_samples) {
			moderror_msg("Number of input samples %d not integer "
			"divisible by the frame period of %d radio frames.\n",
			rcv_samples, FRAME_PERIOD);
			return -1;
		}

		empty = 0;
		subframe = 0;
		frame = 0;

		for (i=0; i<FRAME_PERIOD; i++) {
			memcpy(outi[i], &input[i*samples_per_period], samples_per_period);
		}

	}

	if (subframe == 10*frame) {
		memcpy(output, outi[frame], samples_per_period);

		frame++;
#ifdef _COMPILE_ALOE
		moddebug("frame = %d ts=%d\n", frame,oesr_tstamp(ctx));
#endif
		snd_samples = samples_per_period;
	}

	subframe++;

	if (frame==FRAME_PERIOD) {
		empty = 1;
	}

#ifdef _COMPILE_ALOE
	moddebug("Sent %d samples at ts=%d\n",snd_samples,oesr_tstamp(ctx));
#endif

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

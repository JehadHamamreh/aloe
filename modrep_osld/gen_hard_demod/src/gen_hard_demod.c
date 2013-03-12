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

#include "gen_hard_demod.h"
#include "hard_demod.h"

pmid_t modulation_id;



/**
 * @ingroup gen_hard_demod
 * Initializes parameters.
 *
 * Modulation types: BPSK, QPSK, QAM16, QAM64.
 *
 * \param modulation Modulation index (0: BPSK, 1: QPSK, 2: QAM16, 3: QAM64).
 * Default: 1 (QPSK).
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	/* obtains a handler for fast access to the parameter */
	modulation_id = param_id("modulation");

	return 0;
}


/**
 * @ingroup gen_hard_demod
 *
 * Main DSP function
 *
 * Calls the appropriate function for demodulating all recevied symbols.
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
 *
 *
 */
int work(void **inp, void **out) {
	int i;
	int rcv_samples, snd_samples;
	int modulation;
	float sigma2;
	int bits_per_symbol;
	input_t *input;
	output_t *output;

	if (!get_input_samples(0)) {
		return 0;
	}

	/* Dynamically obtain modulation type */
	if (param_get_int(modulation_id, &modulation) != 1) {
		moderror("Error getting 'modulation' parameter. Assuming QPSK.\n");
		modulation = QPSK;
	}

	/* Verify parameters */
	if (modulation > 3 || modulation < 0) {
		moderror_msg("Invalid modulation %d. Specify 1 for BPSK, 2 for QPSK,"
				"3 for 16QAM, or 4 for 64QAM\n", modulation);
		return -1;
	}

	input = inp[0];
	output = out[0];
	rcv_samples = get_input_samples(0); /* number of input samples */
	bits_per_symbol = get_bits_per_symbol(modulation);
	hard_demod(input, output, rcv_samples, modulation);
	snd_samples = rcv_samples*bits_per_symbol;

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

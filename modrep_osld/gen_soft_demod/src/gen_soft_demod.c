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

#include "gen_soft_demod.h"
#include "soft_demod.h"

pmid_t modulation_id, sigma2_id;
int soft;

struct constellation_tables tables;
struct Sx Sx;


/**
 * @ingroup gen_soft_demod
 * Initializes the soft demodulation parameters: constellation tables and soft
 * demodulation auxiliary matrices.
 *
 * Modulation types: BPSK, QPSK, QAM16, QAM64.
 * Demodulation modes: hard (not implemented), approximate log-likelihood
 * ratio (LLR), exact LLR (not implemented)
 *
 * \param modulation Modulation index (0: BPSK, 1: QPSK, 2: QAM16, 3: QAM64).
 * Default: 1 (QPSK).
 * \param soft Soft demodulation indication (0: exact LLR, 1: approximate LLR).
 * Default: 1 (approximate LLR).
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	/* obtains a handler for fast access to the parameter */
	modulation_id = param_id("modulation");
	sigma2_id = param_id("sigma2");
	/* Obtains the parameter value directly */
	if (param_get_int_name("soft", &soft)) {
		soft = 1; /* approx_llr by default */
	}

	/* use this function to print formatted messages */
	if (soft == 0) {
		modinfo_msg("Soft Demodulation mode %d (exact LLR).\n", soft);
	} else {
		modinfo_msg("Soft Demodulation mode %d (approximate LLR)\n",soft);
	}
	/* Only approximate LLR soft demodulation algorithm implemented so far */
	if ((soft < 0) || (soft > 1)) {
		moderror_msg("Wrong soft demodulation mode %d. Specify 0 for "
			"exact and 1 for approximate LLR algorithm.\n", soft);
		return -1;
	}
	/* set tables and matrices for soft demodulation */
	set_BPSKtable(tables.bpsk, Sx.bpsk);
	set_QPSKtable(tables.qpsk, Sx.qpsk);
	set_16QAMtable(tables.qam16, Sx.qam16);
	set_64QAMtable(tables.qam64, Sx.qam64);

	return 0;
}


/**
 * @ingroup gen_soft_demod
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
	int rcv_samples, snd_samples;
	int modulation;
	float sigma2;
	int bits_per_symbol;
	input_t *input;
	output_t *output;

	/* Dynamically obtain demodulation parameters */
	if (param_get_int(modulation_id, &modulation) != 1) {
		moderror("Error getting 'modulation' parameter\n");
		return -1;
	}
	if (param_get_float(sigma2_id, &sigma2) != 1) {
		moderror("Error getting noise variance (sigma2) parameter\n");
		return -1;
	}

	/* Verify parameters */
	if (modulation > 3 || modulation < 0) {
		moderror_msg("Invalid modulation %d. Specify 0 for BPSK, 1 for QPSK,"
				"2 for 16QAM, or 3 for 64QAM\n", modulation);
		return -1;
	}
	if (sigma2 < 0) {
		moderror_msg("Noise variance %f. Must be greater than 0.\n", sigma2);
		return -1;
	}

	input = inp[0];
	output = out[0];
	rcv_samples = get_input_samples(0); /* number of input samples */
	bits_per_symbol = get_bits_per_symbol(modulation);
	if (soft == 0) {
		switch (modulation) {
			case BPSK:
				llr_exact(input, output, rcv_samples, 2,
				bits_per_symbol, tables.bpsk, Sx.bpsk, sigma2);
				break;
			case QPSK:
				llr_exact(input, output, rcv_samples, 4,
				bits_per_symbol, tables.qpsk, Sx.qpsk, sigma2);
				break;
			case QAM16:
				llr_exact(input, output, rcv_samples, 16,
				bits_per_symbol, tables.qam16, Sx.qam16, sigma2);
				break;
			case QAM64:
				llr_exact(input, output, rcv_samples, 64,
				bits_per_symbol, tables.qam64, Sx.qam64, sigma2);
				break;
		}
	} else { 
		switch (modulation) {
			case BPSK:
				llr_approx(input, output, rcv_samples, 2,
				bits_per_symbol, tables.bpsk, Sx.bpsk, sigma2);
				break;
			case QPSK:
				llr_approx(input, output, rcv_samples, 4,
				bits_per_symbol, tables.qpsk, Sx.qpsk, sigma2);
				break;
			case QAM16:
				llr_approx(input, output, rcv_samples, 16,
				bits_per_symbol, tables.qam16, Sx.qam16, sigma2);
				break;
			case QAM64:
				llr_approx(input, output, rcv_samples, 64,
				bits_per_symbol, tables.qam64, Sx.qam64, sigma2);
				break;
		}
	}
	snd_samples = rcv_samples*bits_per_symbol;
	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

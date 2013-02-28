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

#include "lte_descrambling.h"
#include "descrambling.h"

pmid_t subframe_id;
unsigned c[MAX_c][10];	/* scrambling sequence for all 10 subframes */


/**
 * @ingroup lte_descrambling
 * Verify initialization parameters
 * Verifies the scrambling sequence initialization parameters and returs 0 if
 * all parameter values are correct and -1 if any of the parameters has been
 * wrongly chosen.
 */
int verify_params(struct scrambling_params params) {

	if ((params.q<0) || (params.q>1)) {
		moderror_msg("Wrong codeword index (%d). Choose 0 or 1.\n",
			params.q);
		return -1;
	}
	if ((params.nrnti<0) || (params.nrnti>65535)) {
		moderror_msg("Wrong nRNTI (%d). Choose integer value between 0"
			"and 65535.\n",params.nrnti);
		return -1;
	}
	if ((params.cell_gr<0) || (params.cell_gr>167)) {
		moderror_msg("Wrong physical-layer cell-identity group (%d). "
			"Choose integer value between 0 and 167\n",params.cell_gr);
		return -1;
	}
	if ((params.cell_sec<0) || (params.cell_sec>2)) {
		moderror_msg("Wrong physical-layer identity within the "
			"physical-layer identity group (%d). Choose 0, 1, or 2."
			"\n",params.cell_sec);
		return -1;
	}
	if ((params.channel < 0) || (params.channel > 5)) {
		moderror_msg("Wrong channel type (%d). "
		  "Choose 0 for PDSCH or PUSCH, 1 for PCFICH, 2 for PDCCH, "
		  "3 for PBCH, 4 for PMCH, 5 for PUCCH.\n",params.channel);
		return -1;
	}
	if (params.channel == 3) {
		moderror_msg("Scrambling for PBCH (%d) not yet implemented.\n",
		  params.channel);
		return -1;
	} else {
		return 0;
	}
}


/**
 * @ingroup lte_descrambling
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

	struct scrambling_params params;

	/* get parameters and assign defaults if not (correctly) recevied */
	if (param_get_int_name("q", &params.q)) {
		moderror_msg("Codeword index not specified. Assuming default "
			"value %d.\n",q_default);
		params.q = q_default;
	}
	if (param_get_int_name("cell_gr", &params.cell_gr)) {
		moderror_msg("Cell ID group index (cell_gr) not specified. "
		  "Assuming default value %d.\n", cell_gr_default);
		params.cell_gr = cell_gr_default;
	}
	if (param_get_int_name("cell_sec", &params.cell_sec)) {
		moderror_msg("Cell ID sector index (cell_sec) not specified. "
		  "Assuming default value %d.\n", cell_sec_default);
		params.cell_sec = cell_sec_default;
	}
	if (param_get_int_name("nrnti", &params.nrnti)) {
		moderror_msg("Radio network temporary identifier (nrnti) not "
		  "specified. Assuming default value %d.\n", nrnti_default);
		params.nrnti = nrnti_default;
	}
	if (param_get_int_name("nMBSFN", &params.nMBSFN)) {
		moderror_msg("nMBSFN not specified. Assuming default value %d."
		  "\n", nMBSFN_default);
		params.nMBSFN = nMBSFN_default;
	}
	if (param_get_int_name("channel", &params.channel)) {
		moderror_msg("Physical channel type (channel) not specified. "
		  "Choose 0 for PDSCH or PUSCH, 1 for PCFICH, 2 for PDCCH, 3 "
		  "for PBCH, 4 for PMCH, 5 for PUCCH. Assuming default value "
		  "%d.\n", channel_default);
		params.channel = channel_default;
	}
	params.Nc = Nc_default;	/* fixed value */

	/* Verify parameters */
	if (verify_params(params))
		return -1;

	/* Obtain a handler for fast access to the parameter */
	subframe_id = param_id("subframe");

	/* Generate scrambling sequence based on above parameters, assuming
	 * that it does not change while performing the DSP. */
	sequence_generation(c, params);
	return 0;
}


/**
 * @ingroup lte_descrambling
 *
 * Main DSP function
 *
 * Calls the appropriate function for scrmabling all recevied bits with appropriate scrambling sequence.
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
	static int subframe = -1;
	input_t *input;
	output_t *output;

	rcv_samples = get_input_samples(0);
	if (!rcv_samples) {
		return 0;
	}

	/* If subframe number is not obtained, increment internally by 1
	 * at each invocation (assuming a time slot of 1 ms) */
	/* Caution: If passed as parameter, should be passed only once or
	 * externally incremented accordingly */
	if (param_get_int(subframe_id, &subframe) != 1) {
		if (subframe ==9) {
			subframe = 0;
		} else {
			subframe++;
		}
	}
	/* Verify parameters */
	if (subframe < 0 || subframe > 9) {
		moderror_msg("Invalid subframe number %d. Valid values: 0, 1, "
			"2, ..., 9\n", subframe);
		return -1;
	}

	input = inp[0];
	output = out[0];

	/* scramble with c */
	j = 1;
	s = 0;
	for (i=0; i<rcv_samples; i++) {
		if (i == j*32) {
			s = 0;
			j++;
		}
		if ((((input[i] > 0) && ((c[j-1][subframe]>>s)&1))
			|| ((input[i] <= 0) && ((c[j-1][subframe]>>s)&1)))) {
	                output[i] = -input[i];
		} else {
			output[i] = input[i];
		}
		s++;
	}

	snd_samples = rcv_samples;
	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

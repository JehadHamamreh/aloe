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
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_scrambling.h"
#include "scrambling.h"

pmid_t subframe_id, sample_id;
unsigned c[NOF_SUBFRAMES][MAX_c]; /* scrambling sequence for all subframes */
int direct; 	/* if set directly scrambles the input (chars) */
int ul; 	/* if set, searches for ACK/NACK or Rank Indication repetition
placeholder bits processed in the LTE UL */
int pbch;
int hard;

extern int input_sample_sz;
extern int output_sample_sz;

/**
 * @ingroup Verify initialization parameters
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
		moderror_msg("Wrong nRNTI (%d). Choose integer value between 0 "
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
		pbch = 1;
		direct = 1;
	} else {
		pbch = 0;
	}

	return 0;
}


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

	struct scrambling_params params;

	/* get parameters and assign defaults if not (correctly) recevied */
	if (param_get_int_name("hard", &hard)) {
		moddebug("Scrambling type not specified. Assuming "
		  "bit-scrambling: hard = %d.\n",1);
		hard = 1;
	}
	if (param_get_int_name("q", &params.q)) {
		moddebug("Codeword index not specified. Assuming default "
			"value %d.\n",q_default);
		params.q = q_default;
	}
	if (param_get_int_name("cell_gr", &params.cell_gr)) {
		moddebug("Cell ID group index (cell_gr) not specified. "
		  "Assuming default value %d.\n", cell_gr_default);
		params.cell_gr = cell_gr_default;
	}
	if (param_get_int_name("cell_sec", &params.cell_sec)) {
		moddebug("Cell ID sector index (cell_sec) not specified. "
		  "Assuming default value %d.\n", cell_sec_default);
		params.cell_sec = cell_sec_default;
	}
	if (param_get_int_name("nrnti", &params.nrnti)) {
		moddebug("Radio network temporary identifier (nrnti) not "
		  "specified. Assuming default value %d.\n", nrnti_default);
		params.nrnti = nrnti_default;
	}
	if (param_get_int_name("nMBSFN", &params.nMBSFN)) {
		moddebug("nMBSFN not specified. Assuming default value %d."
		  "\n", nMBSFN_default);
		params.nMBSFN = nMBSFN_default;
	}
	if (param_get_int_name("channel", &params.channel)) {
		moddebug("Physical channel type (channel) not specified. "
		  "Choose 0 for PDSCH or PUSCH, 1 for PCFICH, 2 for PDCCH, 3 "
		  "for PBCH, 4 for PMCH, 5 for PUCCH. Assuming default value "
		  "%d.\n", channel_default);
		params.channel = channel_default;
	}
	params.Nc = Nc_default;	/* fixed value */
	if (param_get_int_name("ul", &ul)) {
		moddebug("Parameter 'ul' not set. Assuming downlink "
		  "operation (%d).\n", 0);
		ul = 0;
	}
	if (param_get_int_name("direct", &direct)) {
		moddebug("Direct parameter not set (%d). This transforms "
		  "input bits from chars to integers, performs descrambling and "
		  "converts result back to chars. Set if processing short "
		  "input sequences to directly scramble input samples.\n", 0);
		direct = 0;
	}

	/* Verify parameters */
	if (verify_params(params))
		return -1;

	if (hard) { /* bit scrambling */
		input_sample_sz=sizeof(char);
		output_sample_sz=sizeof(char);
	} else { /* soft-bit scrambling */
		input_sample_sz=sizeof(float);
		output_sample_sz=sizeof(float);
	}

	/* Obtain a handler for fast access to the parameter */
	subframe_id = param_id("subframe");
	sample_id = param_id("sample");


	/* Generate scrambling sequence based on above parameters, assuming
	 * that they do not change during runtime. */
	sequence_generation(c, params);
	return 0;
}


/**
 * @ingroup template
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

	int rcv_samples, snd_samples;
	static int subframe = -1;
	int sample;
	char *input_b;
	char *output_b;
	float *input_f;
	float *output_f;

	struct ul_params uparams;


	rcv_samples = get_input_samples(0);
	if (!rcv_samples) {
#ifdef _COMPILE_ALOE
		moddebug("ts=%d rcv_samples=%d.\n",oesr_tstamp(ctx),rcv_samples);
#endif
		return 0;
	}

	input_b = inp[0];
	input_f = inp[0];
	output_b = out[0];
	output_f = out[0];

	if (pbch) {
		subframe = 0;
		/* Frame number needed for PBCH. If not obtained externally,
		 * increments by one on each invocation (with data), assuming
		 * one invocatione each radio frame */
		if (param_get_int(sample_id, &sample) != 1) {
			sample = 0;
		}
		/* verify parameter */
		if ((sample < 0) || (sample > 32*MAX_c-rcv_samples)) {
			moderror_msg("Invalid sample value %d. "
			"Valid values: 0, 1, 2, ...%d\n", sample, MAX_c-rcv_samples);
			return -1;
		}
	} else {
		sample = 0;
		/* If subframe number is not obtained, increment internally by 1
		 * at each invocation (assuming a time slot of 1 ms) */
		/* Caution: If passed as parameter, should be passed only once or
		 * externally incremented accordingly */
		if (param_get_int(subframe_id, &subframe) != 1) {
			if (subframe == 9) {
				subframe = 0;
			} else {
				subframe++;
			}
		}
		/* verify parameter */
		if (subframe < 0 || subframe > 9) {
			moderror_msg("Invalid subframe number %d. "
			"Valid values: 0, 1, 2, ..., 9\n", subframe);
			return -1;
		}
	}

#ifdef _COMPILE_ALOE
	moddebug("ts=%d subframe=%d\n",oesr_tstamp(ctx),subframe);
#endif

	snd_samples = rcv_samples;
	if (hard) { /* bits (scrambling, hard descrambling) */
		#ifdef _COMPILE_ALOE
			moddebug("ts=%d rcv_samples=%d bits (char)\n",oesr_tstamp(ctx),rcv_samples);
		#endif
		if (ul) { /* Check for placeholder bits */
			identify_xy(input_b, rcv_samples, &uparams);
		}
		scramble(input_b, output_b, rcv_samples, c[subframe], direct, sample);
		moddebug("snd_samples=%d bits (char)\n.", snd_samples);
	} else { /* soft bits, i.e., floats (descrambling) */
		#ifdef _COMPILE_ALOE
			moddebug("ts=%d rcv_samples=%d soft 'bits' (float)\n",oesr_tstamp(ctx),rcv_samples);
		#endif
		soft_scrambling(input_f, output_f, rcv_samples, c[subframe], sample);
		moddebug("snd_samples=%d soft 'bits' (float)\n.", snd_samples);
	}

	if (ul) {
		/* Set placeholder bits to values indicated by 3GPP */
		set_xy(output_b, uparams);
	}

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

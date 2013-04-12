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
#include <string.h>
#include <stdlib.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_crc_scrambling.h"
#include "crc_scrambling.h"

pmid_t rnti_id, antenna_selection_id, dci_format_id, ue_port_id, nof_ports_id;

int L; 		/* crc length to be scrambled */
int channel; 	/* specifies LTE control channel, as each applies another scrambling sequence */
int direction;	/* specifies if Tx/bit-scrambling (0) or Rx/soft-bit scrambling (!=0) */

/**
 * @ingroup Verify initialization parameters
 * Verifies the scrambling sequence initialization parameters and returs 0 if
 * all parameter values are correct and -1 if any of the parameters has been
 * wrongly chosen.
 */
int verify_initialization_parameters(void)
{
	if ((L<1) || (L>L_MAX)) {
		moderror_msg("Wrong or unsupported CRC length %d. CRC length "
			"must be a positive integer between 1 and %d.\n",
			L, L_MAX);
		return -1;
	}
	if ((channel<0) || (channel>1)) {
		moderror_msg("Unknown channel type (%d). Choose 0 for PDCCH or "
			"1 for PBCH.\n",channel);
		return -1;
	}

	return 0;
}

inline int verify_runtime_parameters(unsigned rnti, int ue_port)
{
	if (L != 16) {
		moderror_msg("Wrong crc length %d specified. Should be 16.\n", L);
		return -1;
	}
	if ((rnti < 0) || (rnti > 0xFFFF)) {
		moderror_msg("Wrong rnti %d. Should be lager than 0 and less "
		  "than 0xFFFF.\n", rnti);
		return -1;
	}
	if ((ue_port < 0) || (ue_port > 1)) {
		moderror_msg("Wrong UE port %d. Should be either 0 or 1.\n", ue_port);
		return -1;
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

	/* get parameters and assign defaults if not (correctly) recevied */
	if (param_get_int_name("direction", &direction)) {
		moderror_msg("Scrambling direction not specified. Assuming "
		  "%d: transmitter.\n", 0);
		direction = 0;
	}
	if (param_get_int_name("crc_length", &L)) {
		moderror_msg("Number of parity bits not specified. Assuming "
		  "default value %d.\n", L_default);
		L = L_default;
	}
	if (param_get_int_name("channel", &channel)) {
		moderror_msg("LTE channel not specified. Assuming (%d) - "
		  "PDCCH.\n", PDCCH);
		channel = channel_default;
	}
	/* Obtain a handler for fast access to the parameter */
	rnti_id = param_id("rnti");
	antenna_selection_id = param_id("antenna_selection");
	dci_format_id = param_id("dci_format");
	ue_port_id = param_id("ue_port");
	nof_ports_id = param_id("nof_ports");	/* PBCH */

	/* Verify parameters */
	if (verify_initialization_parameters())
		return -1;

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
	int i;
	int rcv_samples, snd_samples;
	char *input_b;
	char *output_b;
	float *input_f;
	float *output_f;
	char crc_b[L_MAX], out_crc_b[L_MAX];
	float crc_f[L_MAX], out_crc_f[L_MAX];
	char c[L_MAX];
	struct pdcch_params params;
	int nof_ports;

	rcv_samples = get_input_samples(0);
	if (!rcv_samples) {
		return 0;
	}
	snd_samples = rcv_samples;

	input_f = inp[0];
	input_b = inp[0];
	output_f = out[0];
	output_b = out[0];

	if (channel == 0) {	/* PDCCH */
		if (param_get_int(rnti_id, &params.rnti) != 1) {
			moderror("Error getting 'rnti' parameter. Assuming 0.\n");
			params.rnti = 0;
		}
		if (param_get_int(antenna_selection_id, &params.antenna_selection) != 1) {
			moderror("Error getting 'antenna_selection' parameter. Assuming 0.\n");
			params.antenna_selection = 0;
		}
		if (param_get_int(dci_format_id, &params.dci_format) != 1) {
			moderror("Error getting 'dci_format'. Assuming 0.\n");
			params.dci_format = 0;
		}
		if (param_get_int(ue_port_id, &params.ue_port) != 1) {
			moderror("Error getting 'ue_port'. Assuming port 0\n");
			params.ue_port = 0;
		}
		/* Verify parameters */
		if (verify_runtime_parameters(params.rnti, params.ue_port)) {
			return -1;
		}
		/* Generate scrambling sequence */
		pdcch_sequence_gen(c, params);

	} else {	/* PBCH */
		if (param_get_int(nof_ports_id, &nof_ports) != 1) {
			moderror("Error getting 'nof_ports' parameter. Assuming 1.\n");
			nof_ports = 1;
		} else {
			if ((nof_ports == 1) || (nof_ports == 2) || (nof_ports == 4)) {
				pbch_sequence_gen(c, nof_ports);
			} else {
				moderror_msg("Wrong number of antenna ports "
				  "specified %d. Choose 1, 2, or 4.\n", nof_ports);
				return -1;
			}
		}
	}

	if (!direction) {
		/* Transmitter (bit-scrambling) */
		memcpy(output_b, input_b, rcv_samples-L);
		srambling(&input_b[rcv_samples-L], &output_b[rcv_samples-L], L, c);
	} else {
		/* Receiver (soft-bit scrambling) */
		memcpy(output_f, input_f, rcv_samples-L);
		desrambling(&input_f[rcv_samples-L], &output_f[rcv_samples-L], L, c);
	}

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

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
#include <oesr.h>
#include <params.h>
#include <skeleton.h>
#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "crc_scrambling.h"



/**
 * @ingroup LTE scrambling sequence generator for scrambling CRC parity bits
 * of PDCCH. Generates the scrambling sequence based on the 3GPP specification.
 *
 * \param c Scrambling sequence
 * \param params Parameters that define the pdcch scrambling sequence
 */
inline void pdcch_sequence_gen(char *c, struct pdcch_params params)
{
	int i;
	int L = 16;

	memset(c, 0, L);
	if ((params.antenna_selection == 0) || (params.dci_format == 0)) {
		/* antenna selection not configured or applicable || dci format 0 */
		for (i=0; i<L; i++) {
			c[i] = (params.rnti>>(L-i-1))&1;
		}
	}
	if ((params.antenna_selection > 0) &&  (params.dci_format == 0)) {
	        /* antenna selection configured and applicable && dci format 0 */
		if (c[L-1] == (params.ue_port & 1)) {
			c[L-1] = 0x0;
		} else {
			c[L-1] = 0x1;
		}
	}
}

/**
 * @ingroup LTE scrambling sequence generator for scrambling CRC parity bits
 * of PBCH. Generates the scrambling sequence based on the 3GPP specification.
 *
 * \param c Scrambling sequence
 * \param nof_port Number of antenna ports
 */
inline void pbch_sequence_gen(char *c, int nof_ports)
{
	int i;
	int L = 16;

	memset(c, 0, L); 	/* nof_ports = 1 */
	if (nof_ports == 2) {
		memset(c, 1, L);
	} else if (nof_ports == 4) {
		for (i=1; i<L; i+=2) {
			c[i] = 0x1;
		}
	}
}

/**
 * @ingroup Scrambling function
 * Directly scrambles the input bit-sequence (char) with the scrambling
 * sequence (32-bit integer).
 *
 * \param input Pointer to input data, input bit-sequence (chars)
 * \param output Pointer to output data, scrambled bit-sequence (chars)
 * \param N Number of input samples
 * \param c Scrambling sequence
 */
inline void srambling(char *input, char *output, int N, char *c)
{
	int i;

	for (i=0; i<N; i++) {
		if (input[i] == c[i]) {
	                output[i] = 0x0;
		} else {
			output[i] = 0x1;
		}
	}
}

/**
 * @ingroup Descrambling function (soft bits)
 * Directly scrambles the input bit-sequence (floats) with the scrambling
 * sequence (32-bit integers).
 *
 * \param input Pointer to input data, input soft-bit sequence (floats)
 * \param output Pointer to output data, scrambled bit-sequence (floats)
 * \param N Number of input samples
 * \param c Srambling sequence (32 bits max)
 */
inline void desrambling(float *input, float *output, int N, char *c)
{
	int i;

	for (i=0; i<N; i++) {
		if (((input[i] > 0) && c[i] == 0x0)
			|| ((input[i] <= 0) && (c[i] == 0x1))) {
	                output[i] = -input[i];
		} else {
			output[i] = input[i];
		}
	}

}

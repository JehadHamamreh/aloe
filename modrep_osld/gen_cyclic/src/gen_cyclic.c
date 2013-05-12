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

#include "gen_cyclic.h"

pmid_t dft_size_id;
pmid_t cyclic_prefix_sz_id;
pmid_t first_cyclic_prefix_sz_id;
int direction;

/**@ingroup gen_cyclic
 *
 * \param dft_size Size of the OFDM symbol (in samples). This parameter is mandatory.
 * \param cyclic_prefix_sz Size of the cyclic prefix to add to each received symbol (in samples)
 * This parameter is mandatory.
 * \param first_cyclic_prefix_sz Size of the cyclic prefix to add to the first received symbol
 * (in samples). Optional parameter, default is cyclic_prefix_sz
 */
int initialize() {

	dft_size_id = param_id("dft_size");
	if (!dft_size_id) {
/*		moderror("Parameter dft_size undefined\n");
		return -1;*/
		modinfo("Parameter dft_size not defined. Assuming "
		"dft_size = number of input samples on interface 0.\n");
	}

	cyclic_prefix_sz_id = param_id("cyclic_prefix_sz");
	if (!cyclic_prefix_sz_id) {
		moderror("Parameter cyclic_prefix_sz undefined\n");
		return -1;
	}

	first_cyclic_prefix_sz_id = param_id("first_cyclic_prefix_sz");

	if (NOF_INPUT_ITF != NOF_OUTPUT_ITF) {
		moderror_msg("Fatal error, the number of input interfaces (%d) must be equal to the "
				"number of output interfaces (%d)\n",NOF_INPUT_ITF,NOF_OUTPUT_ITF);
		return -1;
	}

	return 0;
}

/**@ingroup gen_cyclic
 *
 * For each interface, the number of received samples must be multiple of the OFDM symbol size,
 * otherwise the module produces and error and stops the waveform.
 *
 * It adds a cyclic prefix to each OFDM symbol received from each interface.
 */
int work(void **inp, void **out) {
	int i, j;
	int nof_ofdm_symb;
	int dft_size, cyclic_prefix_sz, first_cyclic_prefix_sz;
	int k, nof_ofdm_symbols_per_slot,rcv_samples;
	int cnt;
	int cpy;
	div_t symbols;
	input_t *input;
	output_t *output;

	if (param_get_int(dft_size_id, &dft_size) != 1) {
		/*moderror("getting parameter dft_size\n");
		return -1;*/
		dft_size = get_input_samples(0);
		moddebug("Parameter dft_size not defined. Assuming %d"
		" (number of input samples on interface 0).\n", dft_size);
	}

	if (dft_size == 0) {
		modinfo("dft_size = 0. Returning.\n");
		return 0;
	} else {
		moddebug("dft_size = %d.\n", dft_size);
	}

	if (param_get_int(cyclic_prefix_sz_id, &cyclic_prefix_sz) != 1) {
		moderror("getting parameter cyclic_prefix_sz\n");
		return -1;
	}
	if (first_cyclic_prefix_sz_id) {
		if (param_get_int(first_cyclic_prefix_sz_id, &first_cyclic_prefix_sz) != 1) {
			moderror("getting parameter first_cyclic_prefix_sz\n");
			return -1;
		}
	} else {
		first_cyclic_prefix_sz = cyclic_prefix_sz;
	}
	if (first_cyclic_prefix_sz > dft_size || cyclic_prefix_sz > dft_size) {
		moderror_msg("Error with parameters dft_size=%d, cyclic_prefix_sz=%d, "
				"first_cyclic_prefix_sz=%d\n",dft_size,cyclic_prefix_sz,first_cyclic_prefix_sz);
		return -1;
	}

	if (first_cyclic_prefix_sz == cyclic_prefix_sz) {
		nof_ofdm_symbols_per_slot = 6;
	} else {
		nof_ofdm_symbols_per_slot = 7;
	}

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input = inp[i];
		output = out[i];
		rcv_samples = get_input_samples(i);
		moddebug("rcv_len=%d samples \n",rcv_samples);

		symbols = div(rcv_samples,dft_size);

		if (symbols.rem) {
			moderror_msg("Number of input samples (%d) must be "
			"integer multiple of dft_size (%d) on interface "
			"%d\n",rcv_samples,dft_size,i);
			return -1;
		}

		nof_ofdm_symb = symbols.quot;

		if (nof_ofdm_symb) {
			k = 0;
			cnt = 0;
			for (j=0;j<nof_ofdm_symb;j++) {
				if (j == k*nof_ofdm_symbols_per_slot) {
					cpy = first_cyclic_prefix_sz;
					k++;
				} else {
					cpy = cyclic_prefix_sz;
				}
				memcpy(output, &input[dft_size-cpy], sizeof(input_t)*cpy);
				memcpy(&output[cpy], input, sizeof(input_t)*dft_size);
				input += dft_size;
				output += dft_size+cpy;
				cnt += dft_size+cpy;
			}

			set_output_samples(i, cnt);
		}
	}
	return 0;
}

int stop() {
	return 0;
}


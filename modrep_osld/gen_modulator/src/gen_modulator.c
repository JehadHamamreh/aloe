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

#include "gen_modulator.h"
#include "modulator.h"

pmid_t modulation_id;
static int modulation;

extern int output_sample_sz;
static int out_real=0;

int modulate(input_t *input, output_t *output, int nof_bits);
int modulate_real(input_t *input, float *output, int nof_bits);


/**@ingroup gen_modulator
 *
 * \param modulation Choose between 0: BPŜK, 1: QPSK, 2: QAM16 or 3: QAM64. Default is BPSK.
 */
int initialize() {

	modulation_id = param_id("modulation");
	if (!modulation_id) {
		modinfo("Parameter modulation not configured. Setting to BPSK\n");
		modulation = BPSK;
	}

	param_get_int_name("out_real",&out_real);
	if (out_real == 1) {
		output_sample_sz=sizeof(float);
	}
	set_BPSKtable();
	set_QPSKtable();
	set_16QAMtable();
	set_64QAMtable();

	return 0;
}


int work(void **inp, void **out) {
	int i, out_len;
	input_t *input;
	output_t *output;
	float *outreal;

	if (!get_input_samples(0)) {
		return 0;
	}

	param_get_int(modulation_id,&modulation);

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input = inp[i];
		output = out[i];
		outreal = out[i];
		moddebug("rcv_len=%d\n",get_input_samples(i));

		if (get_input_samples(i)) {
			if (out_real) {
				out_len = modulate_real(input,outreal,get_input_samples(i));
			} else {
				out_len = modulate(input,output,get_input_samples(i));
			}
			if (out_len == -1) {
				return -1;
			}
			set_output_samples(i,out_len);
		}
	}
	return 0;
}

int stop() {
	return 0;
}

int modulate_real(input_t *input, float *output, int nof_bits) {
	int bits_per_symbol, nof_symbols;
	int i,j;

	j = 0;
	bits_per_symbol = get_bits_per_symbol(modulation);
	nof_symbols = nof_bits/bits_per_symbol;

	switch(modulation) {
	case BPSK:
		for (i=0; i<nof_bits; i+=bits_per_symbol) {
			modulate_BPSK_real(&input[i], &output[j]);
				j++; // one symbol every bits_per_symbol bits
		}
		break;
	default:
		return -1;
	}
	return nof_symbols;
}

int modulate(input_t *input, output_t *output, int nof_bits) {
	int bits_per_symbol, nof_symbols;
	int i,j;

	j = 0;
	bits_per_symbol = get_bits_per_symbol(modulation);
	nof_symbols = nof_bits/bits_per_symbol;

	switch(modulation) {
	case BPSK:
		for (i=0; i<nof_bits; i+=bits_per_symbol) {
			modulate_BPSK(&input[i], &output[j]);
				j++; // one symbol every bits_per_symbol bits
		}
	break;
	case QPSK:
		for (i=0; i<nof_bits; i+=bits_per_symbol) {
			modulate_QPSK(&input[i], &output[j]);
				j++; // one symbol every bitspersymbol bits
		}
	break;
	case QAM16:
		for (i=0; i<nof_bits; i+=bits_per_symbol) {
			modulate_16QAM(&input[i], &output[j]);
				j++; // one symbol every bitspersymbol bits
		}
	break;
	case QAM64:
		for (i=0; i<nof_bits; i+=bits_per_symbol) {
			modulate_64QAM(&input[i], &output[j]);
				j++; // one symbol every bitspersymbol bits
		}
	break;
	default:
		moderror_msg("Unknown modulation %d\n",modulation);
		return -1;
	}
	return nof_symbols;
}


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

#include "lte_ratematching.h"
#include "ratematching.h"
#include "grid.h"

pmid_t out_len_id,rvidx_id,tslot_idx_id,fft_size_id;
pmid_t mcs_id,cp_long_id,nrb_id;

static int tslot_idx;

int lteslots_x_timeslot;
int direction;
extern int input_sample_sz;
extern int output_sample_sz;

/**
 * @ingroup lte_ratematching
 *  Rate adapter for LTE DL/UL. Code rate is out_len/in_len
 *
 *  There are two ways to configure this module:
 *    - Automatic: Choose tslot_idx, modulation and fft_size to let the module
 *    automatically determine the required output length at each slot, according to the
 *    LTE slotting
 *    - Manual: Set out_len variable to the desired output length.
 *
 *  Automatic mode is selected when parameter out_len is not defined.
 *
 * \param tslot_idx In automatic mode, indicates the tslot index
 * \param mcs In automatic mode, indicates the Modulation and Coding Scheme (MCS)
 * \param nrb In automatic mode, indicates the Number of allocated Resource Blocs
 * \param fft_size In automatic mode, indicates the length of the fft
 * \param cp_is_long In automatic mode, non-null selects long cyclic prefix
 * \param lteslots_x_timeslot In automatic mode, number of lte slots per execution time slot
 * \param out_len Output length
 * \param rv_idx Document this parameter
 * \param direction 0 for the transmitter, 1 for the receiver
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int size;

	out_len_id = param_id("out_len");
	if (!out_len_id) {
		tslot_idx_id = param_id("tslot_idx");
		tslot_idx = 0;

		if (!(fft_size_id = param_id("fft_size"))) {
			moderror("In automatic mode, parameter fft_size must be specified\n");
		}

		if (!(mcs_id = param_id("mcs"))) {
			moderror("In automatic mode, parameter mcs must be specified\n");
		}
		if (!(nrb_id = param_id("nrb"))) {
			moderror("In automatic mode, parameter nrb must be specified\n");
		}
		if (!(cp_long_id = param_id("cp_is_long"))) {
			moderror("In automatic mode, parameter cp_is_long must be specified\n");
		}
		if (param_get_int_name("lteslots_x_timeslot",&lteslots_x_timeslot)) {
			moderror("In automatic mode, parameter lteslots_x_timeslot must be specified\n");
		}
	}

	rvidx_id = param_id("rvidx");

	if (param_get_int_name("direction",&direction)) {
		moderror("Error getting parameter direction\n");
		return -1;
	}

	if (direction) {
		input_sample_sz=sizeof(float);
		output_sample_sz=sizeof(float);
	} else {
		input_sample_sz=sizeof(char);
		output_sample_sz=sizeof(char);
	}

	return 0;
}

/**
 * @ingroup lte_ratematching
 *
 * Main DSP function
 *
 */
int work(void **inp, void **out) {
	int in_len,nof_active_itf,out_len,out_len_block,rvidx;
	int i,j;
	int fft_size, mcs,cp_is_long,nrb;
	char *input_b;
	char *output_b;
	float *input_f;
	float *output_f;

	if (out_len_id) {
		if (param_get_int(out_len_id, &out_len) != 1) {
			moderror("Error getting parameter out_len\n");
			return -1;
		}
	} else {
		if (tslot_idx_id) {
			if (param_get_int(tslot_idx_id, &tslot_idx) != 1) {
				moderror("Error getting parameter tslot_idx\n");
				return -1;
			}
		}
		if (param_get_int(fft_size_id, &fft_size) != 1) {
			moderror("Error getting parameter fft_size\n");
			return -1;
		}
		if (param_get_int(mcs_id, &mcs) != 1) {
			moderror("Error getting parameter mcs\n");
			return -1;
		}
		if (param_get_int(nrb_id, &nrb) != 1) {
			moderror("Error getting parameter nrb\n");
			return -1;
		}
		if (param_get_int(cp_long_id, &cp_is_long) != 1) {
			moderror("Error getting parameter cp_is_long\n");
			return -1;
		}
		if (!direction) {
			out_len = lte_get_psdch_bits_x_slot(lteslots_x_timeslot*tslot_idx, fft_size, cp_is_long, lteslots_x_timeslot)*
					lte_get_modulation_format(mcs);
		} else {
			out_len = lte_get_cbits(mcs,nrb);
		}
		if (out_len == -1) {
			moderror("Error computing bits per slot\n");
			return -1;
		}
	}


	if (rvidx_id) {
		if (param_get_int(rvidx_id, &rvidx) != 1) {
			moderror("Error getting parameter rvidx\n");
			return -1;
		}
	} else {
		rvidx = 0;
	}

	nof_active_itf = 0;
	for (i=0;i<NOF_INPUT_ITF;i++) {
		if (get_input_samples(i))
			nof_active_itf++;
	}
	
	if (!nof_active_itf) {
		return 0;
	}
	
	out_len_block = out_len/nof_active_itf;
	output_f = out[0];
	output_b = out[0];

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input_f = inp[i];
		input_b = inp[i];
		in_len = get_input_samples(i);

		if (in_len && inp[i]) {
			if(!direction) {
				if (char_RM_block(input_b,&output_b[i*out_len_block],in_len,
						out_len_block,rvidx)) {
					return -1;
				}
			} else {
				if (float_UNRM_block(input_f,&output_f[i*out_len_block],in_len,
						out_len_block,rvidx)) {
					return -1;
				}
			}

		}
	}
	tslot_idx++;
	if (tslot_idx == LTE_NOF_SLOTS_X_FRAME/lteslots_x_timeslot) {
		tslot_idx = 0;
	}
	return out_len_block*nof_active_itf;
}

int stop() {
	return 0;
}


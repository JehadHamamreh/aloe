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

#include "bit_packer.h"
#include "base/pack.h"

int direction;

/**
 * @ingroup bit_packer
 * \param direction 0 converts bytes to bits, 1 converts bits into bytes
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	if (param_get_int_name("direction", &direction)) {
		moderror("Missing parameter direction\n");
		return -1;
	}

	return 0;
}

int work(void **inp, void **out) {
	int i;
	int rcv_samples, snd_samples;
	input_t *input = inp[0];
	output_t *output = out[0];

	rcv_samples = get_input_samples(0);
	if (!rcv_samples) {
		return 0;
	}

	if (!direction) {
		snd_samples = rcv_samples*8;
		for (i=0;i<rcv_samples;i++) {
			pack_bits(((unsigned int) input[i]) & 0xFF,&output,8);
		}
	} else {
		snd_samples = rcv_samples/8;
		for (i=0;i<snd_samples;i++) {
			output[i] = (input_t) unpack_bits(&input,8);
		}

	}
	moddebug("rcv=%d, snd=%d\n",rcv_samples,snd_samples);
	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


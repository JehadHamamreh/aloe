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
#include <rtdal.h>
#include <params.h>
#include <skeleton.h>

#include "dac_source.h"

static float _Complex *buffer;

/**
 * @ingroup dac_source
 *
 * \param freq_samp Sets DA converter sampling frequency
 */
int initialize() {
	buffer = rtdal_uhd_buffer(0);
	if (buffer) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * @ingroup dac_source
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	output_t *output;
	int i,j;
	int block_size;

	for (i=0;i<NOF_OUTPUT_ITF;i++) {
		output = out[i];
		block_size = rtdal_uhd_get_block_len(i);
		for (j=0;j<block_size;j++) {
			output[j] = buffer[j];
		}
		set_output_samples(i,block_size);
	}
	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


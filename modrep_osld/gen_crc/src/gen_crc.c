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

#include "gen_crc.h"
#include "crc.h"

pmid_t long_crc_id, poly_id;
static int long_crc;
static int mode;
static unsigned int poly;

/** @ingroup gen_crc gen_crc
 *
 * \param long_crc Length of the CRC (default 24)
 * \param mode 0: add CRC; 1: check last long_crc bits with the teoretical CRC (default ADD)
 * \param poly CRC polynomi, in hexadecimal (default 0x1864CFB)
 */

int initialize() {

	if (param_get_int(param_id("mode"),&mode) != 1) {
		mode = MODE_ADD;
	}

	long_crc_id = param_id("long_crc");
	if (param_get_int(long_crc_id,&long_crc) != 1) {
		modinfo_msg("Parameter long_crc not configured. Set to %d\n",
				DEFAULT_LONG_CRC);
		long_crc = DEFAULT_LONG_CRC;
		long_crc_id = NULL;
	}

	poly_id = param_id("poly");
	if (param_get_int(poly_id,&poly) != 1) {
		poly = DEFAULT_POLY;
		poly_id = NULL;
	}

	return 0;
}

/**@ingroup gen_crc
 * Adds a CRC to every received packet from each interface
 */
int work(void **inp, void **out) {
	int i, n;
	input_t *input;

	if (poly_id) param_get_int(poly_id,&poly);
	if (long_crc_id) param_get_int(long_crc_id, &long_crc);

	for (i=0;i<NOF_INPUT_ITF;i++) {
		if (get_input_samples(i)) {
			moddebug("rcv_len=%d\n",get_input_samples(i));
			memcpy(out[i],inp[i],sizeof(input_t)*get_input_samples(i));
			input = out[i];
			n = icrc(0, input, get_input_samples(i), long_crc, poly, mode == MODE_ADD);
			if (n == -1) {
				moderror("Running icrc\n");
				return -1;
			}
			set_output_samples(i,get_input_samples(i)+long_crc);
		}
	}
	return 0;
}

int stop() {
	return 0;
}


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
#include <unistd.h>
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

/*#define CHECK_ZEROS
*/

unsigned total_errors, total_pkts;
int print_interval,point_interval,cross_on_error;

/** @ingroup gen_crc gen_crc
 *
 * \param long_crc Length of the CRC (default 24)
 * \param direction 0: add CRC; 1: check last long_crc bits with the theoretical CRC (default ADD)
 * \param poly CRC polynomy, in hexadecimal (default 0x1864CFB)
 */

int initialize() {

	if (param_get_int_name("direction",&mode)) {
		mode = MODE_ADD;
	}

	if (param_get_int_name("print_interval",&print_interval)) {
		print_interval = 0;
	}

	if (param_get_int_name("point_interval",&point_interval)) {
		point_interval = 0;
	}

	if (param_get_int_name("cross_on_error",&cross_on_error)) {
		cross_on_error = 0;
	}

	long_crc_id = param_id("long_crc");
	if (param_get_int(long_crc_id,&long_crc) != 1) {
		moddebug("Parameter long_crc not configured. Set to %d\n",
				DEFAULT_LONG_CRC);
		long_crc = DEFAULT_LONG_CRC;
		long_crc_id = NULL;
	}

	poly_id = param_id("poly");
	if (param_get_int(poly_id,(int*) &poly) != 1) {
		poly = DEFAULT_POLY;
		poly_id = NULL;
	}

	total_errors=0;
	total_pkts=0;
	return 0;
}

/**@ingroup gen_crc
 * Adds a CRC to every received packet from each interface
 */
int work(void **inp, void **out) {
	int i;
	unsigned int n;
	int rcv_samples;
	input_t *input;

	if (poly_id) param_get_int(poly_id,(int*)&poly);
	if (long_crc_id) param_get_int(long_crc_id, &long_crc);

	for (i=0;i<NOF_INPUT_ITF;i++) {
		rcv_samples = get_input_samples(i);
		if (rcv_samples) {
			if (out[i]) {
				memcpy(out[i],inp[i],sizeof(input_t)*rcv_samples);
				input = out[i];
			} else {
				input = inp[i];
			}
			n = icrc(0, input, rcv_samples, long_crc, poly, mode == MODE_ADD);

			if (mode==MODE_CHECK) {
#ifdef CHECK_ZEROS
				int j;
				char checkzero;
				checkzero=0;
				for (j=0;j<rcv_samples;j++) {
					checkzero+=input[j];
				}
#ifdef _COMPILE_ALOE
				if (!checkzero) {
					modinfo_msg("received zero packet ts=%d len=%d\n",oesr_tstamp(ctx),rcv_samples)
				}
#endif
#endif
				if (n) {
					total_errors++;
#ifdef _COMPILE_ALOE
					modinfo_msg("error at packet %d ts=%d len=%d\n",total_pkts,oesr_tstamp(ctx),rcv_samples);
#endif
					if (cross_on_error) {
						write(0,"x",1);
					}
				}

				total_pkts++;
				set_output_samples(i,rcv_samples-long_crc);

				if (print_interval && !(total_pkts%print_interval)) {
					modinfo_msg("Total blocks: %d\tTotal errors: %d\tBLER=%g\n",
								total_pkts,total_errors,(float)total_errors/total_pkts);
				}

				if (point_interval && !(total_pkts%point_interval)) {
					write(0,".",1);
				}
			} else {
				set_output_samples(i,rcv_samples+long_crc);
			}
		}
	}
	return 0;
}

int stop() {
	if (mode==MODE_CHECK) {
		modinfo_msg("Total blocks: %d\tTotal errors: %d\tBLER=%g\n",
						total_pkts,total_errors,(float)total_errors/total_pkts);
	}
	return 0;
}


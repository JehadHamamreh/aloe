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

#include "dup.h"

extern int nof_output_itf;

/**
 * @ingroup dup
 *
 * \param nof_output_itf Mandatory. Number of output interfaces the input data will be duplicated to.
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	if (param_get_int_name("nof_output_itf", &nof_output_itf)) {
		moderror("Missing nof_output_itf parameter\n");
		return -1;
	}

	return 0;
}

int work(void **inp, void **out) {
	int rcv_samples = get_input_samples(0);
	int i;

	if (!rcv_samples) {
		return 0;
	}
	for (i=0;i<nof_output_itf;i++) {
		memcpy(out[i],inp[0],rcv_samples);
	}
	return rcv_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


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
#include <skeleton.h>
#include <params.h>

#include "source.h"
#include "lte_lib/grid.h"
#include "generators/generators.h"

pmid_t mcs_id,nrb_id,en_id;
generator_t *g;
int last_block_length;

/**@ingroup lte_tb_source
 *
 * The available generators are defined in generators.h
 * \param mcs Modulation and Coding Scheme
 * \param nrb Number of Resource Blocks
 *
 */
int initialize() {
	int size;
	int i;
	int block_length;

	last_block_length = 0;

	mcs_id = param_id("mcs");
	if (!mcs_id) {
		moderror("Parameter mcs not found\n");
		return -1;
	}
	nrb_id = param_id("nrb");
	if (!nrb_id) {
		moderror("Parameter nrb not found\n");
		return -1;
	}
	en_id = param_id("enabled");

	generator_init_random();

	for(i=0;i<NOF_GENERATORS;i++) {
		if (generators[i].key == GENERATOR_BINARY) {
			break;
		}
	}
	if (i==NOF_GENERATORS) {
		moderror("Binary generator not found in library\n");
		return -1;
	}
	g = &generators[i];

	return 0;
}

int work(void **inp, void **out) {
	int mcs,nrb;
	int block_length;
	int i,j;
	int snd_samples, en;

	if (en_id) {
		if (param_get_int(en_id,&en) == 1) {
			if (en == 0) {
				last_block_length = 0;
				return 0;
			}
		}
	}

	if (param_get_int(mcs_id,&mcs) != 1) {
		moderror("Getting integer parameter mcs\n");
		return -1;
	}
	if (param_get_int(nrb_id,&nrb) != 1) {
		moderror("Getting integer parameter nrb\n");
		return -1;
	}

	block_length = lte_get_tbs(mcs,nrb);
	if (block_length == -1) {
		moderror_msg("Getting block_length for MCS=%d, NRB=%d\n",mcs,nrb);
		return -1;
	}

#ifdef _COMPILE_ALOE
	if (block_length != last_block_length) {
		modinfo_msg("Select block_length: block_length=%d at tslot=%d\n",block_length,oesr_tstamp(ctx));
		last_block_length = block_length;
	}
#endif

	//snd_samples = g->work(out[0],block_length);
	snd_samples = block_length;

	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}




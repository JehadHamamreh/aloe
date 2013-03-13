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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include <skeleton.h>
#include <params.h>

#define INCLUDE_DEFS_ONLY
#include "lte_resource_demapper.h"

#include "lte_lib/grid/base.h"

int block_length;
int offset=0;

extern struct lte_grid_config grid;
extern int nof_input_itf;

/**
 *  Generates input signal. VERY IMPORTANT to fill length vector with the number of
 * samples that have been generated.
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 */
int generate_input_signal(void *in, int *lengths)
{
	int i,n;
	input_t *input = in;
	int subframe_idx=0;
	int nof_pdsch,nof_pdcch,en_pbch,en_pcfich,en_phich;
	int nof_ch;

	if (param_get_int_name("subframe_idx",&subframe_idx)) {
		moderror("Getting integer parameter subframe_idx\n");
		return -1;
	}
	nof_ch=0;
	if (param_get_int_name("en_pbch",&en_pbch)) {
		en_pbch=0;
	}
	if (en_pbch) {
		if (nof_ch >= nof_input_itf) {
			printf("Error only %d interfaces configured\n",nof_input_itf);
			return -1;
		}
		lengths[nof_ch] = lte_ch_get_re(n,CH_PBCH,subframe_idx,&grid);
		printf("Generating PBCH with %d RE\n",lengths[nof_ch]);
		for (i=0;i<PBCH_RE;i++) {
			__real__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) en_pbch;
			__imag__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) en_pbch;
		}
		nof_ch++;
	}


	if (param_get_int_name("en_phich",&en_phich)) {
		en_phich=0;
	}
	if (en_phich) {
		if (nof_ch >= nof_input_itf) {
			printf("Error only %d interfaces configured\n",nof_input_itf);
			return -1;
		}
		lengths[nof_ch] = lte_ch_get_re(0,CH_PHICH,subframe_idx,&grid);
		printf("Generating PHICH with %d RE\n",lengths[nof_ch]);
		for (i=0;i<lengths[nof_ch];i++) {
			__real__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) en_phich;
			__imag__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) en_phich;
		}
		nof_ch++;
	}

	if (param_get_int_name("en_pcfich",&en_pcfich)) {
		en_pcfich=0;
	}
	if (en_pcfich) {
		if (nof_ch >= nof_input_itf) {
			printf("Error only %d interfaces configured\n",nof_input_itf);
			return -1;
		}
		lengths[nof_ch] = lte_ch_get_re(0,CH_PCFICH,subframe_idx,&grid);
		printf("Generating PCFICH with %d RE\n",lengths[nof_ch]);
		for (i=0;i<lengths[nof_ch];i++) {
			__real__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) en_pcfich;
			__imag__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) en_pcfich;
		}
		nof_ch++;
	}

	if (param_get_int_name("nof_pdsch",&nof_pdsch)) {
		nof_pdsch=0;
	}
	printf("Generating %d PDSCH channels\n",nof_pdsch);
	for (n=0;n<nof_pdsch;n++) {
		if (nof_ch >= nof_input_itf) {
			printf("Error only %d interfaces configured\n",nof_input_itf);
			return -1;
		}
		lengths[nof_ch] = lte_ch_get_re(n,CH_PDSCH,subframe_idx,&grid);
		printf("PDSCH#%d has %d RE\n",n,lengths[nof_ch]);
		for (i=0;i<lengths[nof_ch];i++) {
			__real__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) nof_ch+1;
			__imag__ input[nof_ch*INPUT_MAX_SAMPLES+i] = (float) nof_ch+1;
		}
		nof_ch++;
	}

	if (param_get_int_name("nof_pdcch",&nof_pdcch)) {
		nof_pdcch=0;
	}
	printf("Generating %d PDCCH channels\n",nof_pdcch);
	for (n=0;n<nof_pdcch;n++) {
		if (nof_ch >= nof_input_itf) {
			printf("Error only %d interfaces configured\n",nof_input_itf);
			return -1;
		}
		lengths[nof_ch] = lte_ch_get_re(n,CH_PDCCH,subframe_idx,&grid);
		printf("PDCCH#%d has %d RE\n",n,lengths[nof_ch]);
		for (i=0;i<lengths[nof_ch];i++) {
			__real__ input[(nof_ch)*INPUT_MAX_SAMPLES+i] = (float)nof_ch+10;
			__imag__ input[(nof_ch)*INPUT_MAX_SAMPLES+i] = (float)nof_ch+10;
		}
		nof_ch++;
	}



	return 0;
}

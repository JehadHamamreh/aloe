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

#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "lte_cheq.h"
#include "lte_lib/grid/base.h"
#include "equalizer.h"
#include "refSignals.h"

/* Parameters needed by rs generation and mapping */
int fftsize;
int Nid;		// Cell identifier number
int Ndlsym;		// Number of ODFDM symbols in a downlink slot
int p; 			// Antenna Port (0, 1, 2, 3)
int fseg;		// Frame segmentation that is received each processing TS

params_t prms [] = {
		{"fftsize",	&fftsize, 	128},
		{"Nid",		&Nid, 		0},
		{"Ndlsym", 	&Ndlsym, 	7},
		{"p", 		&p, 		0},
		{"fseg", 	&fseg, 		10},
		{NULL, NULL, 0}
};

/* Secondary parameters, calculated during initialization */
complex_t rs [MAXRS];	// Reference signals storage array
int rspos [MAXRS];	// Reference signals positions array storage

/**
 *  Generates input signal. VERY IMPORTANT to fi
--------------------------------------
Avg. processing time 0.011118 msec/frame
*** Plotting signal ***
*** Plotting signal ***

Press ENTER to continue...
 *  ll length vector with the number of
 * samples that have been generated.
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 */
int generate_input_signal(void *in, int *lengths)
{
	int k;
	complex_t realchannel [2048];
	input_t *input = in;
	int block_length;
	int err, i, j;
	int nofdm;
	int	Ndlrb;		// Number of PRBs in a downlink ofdm symbol
	int Ncp;		// Cyclic prefix type (1 for normal CP, 0 for extended CP)

	/* Obtain the configuration parameters */
	i = 0;
	while (prms[i].var != NULL){
		if (param_get_int_name((char*) prms[i].name, prms[i].var)) {
			*prms[i].var = prms[i].value;
			modinfo_msg("Test: Parameter %s not defined, setting default %d\n", prms[i].name, *prms[i].var);
		} else {
			modinfo_msg("Test: Parameter %s is %d\n", prms[i].name, *prms[i].var);
		}
		i++;
	}
	modinfo_msg("Test: Parameters have been loaded!\n", NULL);

	/* Generate secondary parameters */
	// Downlink Resource Blocks (RBs)
	if (fftsize == 128){
		Ndlrb = 6;
	} else if (fftsize == 256){
		Ndlrb = 12;
	} else if (fftsize == 512){
		Ndlrb = 25;
	} else if (fftsize == 1024){
		Ndlrb = 50;
	} else if (fftsize == 1536){
		Ndlrb = 75;
	} else if (fftsize == 2048){
		Ndlrb = 100;
	} else {
		moderror("The fft size exceeds the maximum (2048)\n");
	}

	// Number of OFDM symbols per processing TS
	nofdm = Ndlsym*20/fseg;

	block_length=nofdm*fftsize;
	param_get_int_name("block_length",&block_length);

	// Get the Frame configuration for cyclic prefix
	switch (Ndlsym){
		case 7:
			Ncp = 1;
			break;
		case 6:
			Ncp = 0;
			break;
		default:
			moderror_msg("Bad parameter Ndlsym, only valid 6 and 7 (default)\n", NULL);
			break;
	}

	/* Generate RS and map them */
	err = setRefSignals (	Ndlrb,		// Number of PRBs in a downlink ofdm symbol
					Nid,		// Cell identifier number
					Ncp,		// Cyclic prefix
					Ndlsym,		// Number of ODFDM symbols in a downlink slot
					p, 			// Antenna Port (0, 1, 2, 3)
					rs,			// ReferenceNSUBC signals pointer
					rspos,		// Reference signals positions
					fseg		// Frame segmentation that is received each processing TS
				);
	if (err<0) {	// Check if any error occur
		moderror_msg("Bad parameters input in RS initialization (error %d)\n", err);
	} /*else {
		nrs = err/fseg;
	}*/
#define GUARD	((fftsize-12*Ndlrb)/2)

	/* Generate testing channel */
	for (i=0; i<fftsize; i++){
			/*arg = 2*PI*((float)(i%fftsize)/(float)fftsize);
			*/realchannel [i] = i;/*cosf(arg)+I*sinf(arg);*/
	}
	modinfo_msg("Test: testing channel has been generated.\n", NULL);

	/* Generate test frame */
	k = j = 0;
	for (i = 0; i < block_length; i++){
		/* If reference symbol is allocated */
		if (i == rspos[k]){
			input[i] = rs[k]*realchannel [i%fftsize];
			k++; j++;
		/* If traffic symbol is allocated */
		} else if (i%fftsize>GUARD && i%fftsize<(fftsize-GUARD)){
			input[i] = realchannel [i%fftsize];
		}
		/* If nothing */
		else
			input[i] = 0;
	}

	/** HERE INDICATE THE LENGTH OF THE SIGNAL */
	lengths[0] = block_length;

	modinfo_msg("Test: testing frame size %d has been generated.\n", lengths[0]);

	return 0;
}

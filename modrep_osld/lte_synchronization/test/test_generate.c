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
#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <skeleton.h>
#include <params.h>

#define INCLUDE_DEFS_ONLY
//#include "dft.h"		//TREURE*/
#include "syncSignals.h"
#include "utils.h"
#include "syncProcess.h"
#include "noise.h"
#include "lte_synchF.h"

int offset=100;


#define MAXDATASIZE	200*2048
#define CORRFFT	512

_Complex float frameGrid[140*2048];
float  noiseI[MAXDATASIZE], noiseQ[MAXDATASIZE];
fftwf_complex AUXtime0[MAXDATASIZE];
fftwf_complex AUXtime1[MAXDATASIZE];

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
	int i, j, s;
	input_t *input = in;
	int block_length;
	pmid_t blen_id;
	int FFTsize;
	float EbNodBs;
	int nofbitsDA;
	/**PSS*/
	int cellID;
	int delaybetweenPSS=0;
	int firstPSS;
	/**FFT*/
	float maxval;
	int CPsize, diffCPs;

	/**PROGRAM*/
	blen_id = param_id("block_length");
	if (!blen_id) {
		moderror("Parameter block_length not found\n");
		return -1;
	}
	if (!param_get_int(blen_id,&block_length)) {
		moderror("Getting integer parameter block_length\n");
		return -1;
	}

	//Select FFTsize
	FFTsize=128;

	/** Initialize vectors*/
	memset(AUXtime0, 0, sizeof(fftwf_complex)*(MAXDATASIZE));
	memset(AUXtime1, 0, sizeof(fftwf_complex)*(MAXDATASIZE));
	memset(noiseI, 0, sizeof(float)*(MAXDATASIZE));
	memset(noiseQ, 0, sizeof(float)*(MAXDATASIZE));

	//Generate PSS sequence to be send
	genPSStime_seq(0, FFTsize, AUXtime0, -1);

	//Add CP
	CPsize=CP_NOR_Slot1(FFTsize);
	printf("CPsize=%d\n", CPsize);
	//Make space before
	for(i=0; i<FFTsize; i++){
		AUXtime0[FFTsize-1+CPsize-i]=AUXtime0[FFTsize-1-i];
		//printf("FFTsize-1+CPsize-i=%d FFTsize-1-i=%d\n", FFTsize-1+CPsize-i, FFTsize-1-i);
	}

	//Copy back to head
	for(i=0; i<CPsize; i++){
		AUXtime0[CPsize-1-i]=AUXtime0[FFTsize-1+CPsize-i];
		//printf("CPsize-1-i=%d FFTsize-1+CPsize-i=%d\n", CPsize-1-i, FFTsize-1+CPsize-i);
	}

	//Normalize data
	normCvector(AUXtime0, FFTsize+CPsize);
	//Add noise
	EbNodBs=10.0;
	addnoise(EbNodBs, MAXDATASIZE, noiseI);
	addnoise(EbNodBs, MAXDATASIZE, noiseQ);
	//Generate output
	firstPSS=120;		//OFFSET
	printf("firstPSS=%d\n", firstPSS);
	delaybetweenPSS = 10*SIZEOF_SLOT_CPNOR(FFTsize);//17920;	//CP normal, FFTsize=512
	printf("delaybetweenPSS = %d\n", delaybetweenPSS);

	for(i=0; i<block_length; i++){
		__real__ AUXtime1[i] = noiseI[i];
		__imag__ AUXtime1[i] = noiseQ[i];
	}
	j=firstPSS;
	CPsize=CP_NOR_SlotN(FFTsize);
	diffCPs=CP_NOR_Slot1(FFTsize)-CP_NOR_SlotN(FFTsize);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+delaybetweenPSS;
	printf("firstPSS+delaybetweenPSS=%d\n", firstPSS+delaybetweenPSS);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+2*delaybetweenPSS;
	printf("firstPSS+2*delaybetweenPSS=%d\n", firstPSS+2*delaybetweenPSS);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+3*delaybetweenPSS;
		printf("firstPSS+3*delaybetweenPSS=%d\n", firstPSS+3*delaybetweenPSS);
		for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
			__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
			__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+4*delaybetweenPSS;
		printf("firstPSS+4*delaybetweenPSS=%d\n", firstPSS+4*delaybetweenPSS);
		for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
			__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
			__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+5*delaybetweenPSS;
		printf("firstPSS+5*delaybetweenPSS=%d\n", firstPSS+5*delaybetweenPSS);
		for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
			__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
			__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+6*delaybetweenPSS;
		printf("firstPSS+6*delaybetweenPSS=%d\n", firstPSS+6*delaybetweenPSS);
		for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
			__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
			__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+7*delaybetweenPSS;
		printf("firstPSS+7*delaybetweenPSS=%d\n", firstPSS+7*delaybetweenPSS);
		for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
			__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
			__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+8*delaybetweenPSS;
		printf("firstPSS+8*delaybetweenPSS=%d\n", firstPSS+8*delaybetweenPSS);
		for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
			__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
			__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}

	//Quantify input data to 16 bits
	nofbitsDA=16;
	quantifyCvectorGO(AUXtime1, input, block_length, nofbitsDA);


	// HERE INDICATE THE LENGTH OF THE SIGNAL
	lengths[0] = block_length;

	return 0;
}

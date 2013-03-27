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
#include "syncSignals.h"
#include "utils.h"
#include "syncProcess.h"
#include "noise.h"
#include "lte_synchG.h"

#define MAXDATASIZE	200*2048
#define CORRFFT	512
#define DFT_MIRROR		1
#define div(a,b) ((a-1)/b+1)

int offset=100;
_Complex float checkout[MAXDATASIZE];
_Complex float frameGrid[140*2048];

//SSS
extern int ss[N], cc[N], zz[N];


inline static void copy_in(char *dst, char *src, int size_d, int len, int options) {
	int hlen;
	if (options & DFT_MIRROR) {
		//hlen = div(len,2);
		hlen=len/2;
		printf("hlen=%d\n", hlen);
		memcpy(dst, &src[hlen*size_d], size_d*hlen);
		memcpy(&dst[hlen*size_d], src, size_d*(len - hlen));
	} else {
		memcpy(dst,src,size_d*len);
	}
}



float  noiseI[MAXDATASIZE], noiseQ[MAXDATASIZE];
fftwf_complex AUXtime0[MAXDATASIZE];
fftwf_complex AUXtime1[MAXDATASIZE];
_Complex float SSSsubfr0time[2048];
_Complex float SSSsubfr5time[2048];

int m0s[NUMSSS];
int m1s[NUMSSS];
float SSSsf0[SSSLEN+2];
float SSSsf5[SSSLEN+2];

fftwf_complex FFTin[2048], FFTout[2048], src[2048], dst[2048];
fftwf_plan plan128B, plan128F;

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
	int i, j, s, m;
	input_t *input = in, *checkout;
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
	////////////////SSS vars
	int phyLayerCellID;
	int RxphyLayerCellID;
	int nofsubframe;

	//PROVES FFTS
	int len, hlen;
	float gain;


/*	plan128B = fftwf_plan_dft_1d(128, FFTin,FFTout, FFTW_BACKWARD, FFTW_ESTIMATE);
	plan128F = fftwf_plan_dft_1d(128, FFTout,input, FFTW_FORWARD, FFTW_ESTIMATE);
	//fftwf_execute(plan128);


	for(i=0; i<128; i++){
		//if(i < 10 || i > 128-10){
		if(i > 32 && i < 128-32){
			__real__ FFTin[i] = 1.0;//(float)(i);
			__imag__ FFTin[i] = 0.0;
			if(i==64){
				__real__ FFTin[i] = 0.0;
				__imag__ FFTin[i] = 0.0;
			}
		}
		else{
			__real__ FFTin[i] = 0.0;
			__imag__ FFTin[i] = 0.0;
		}
	}
*/
//	for(i=0; i<128; i++)input[i]=FFTin[i];


//	fftwf_execute(plan128B);
/*	for(i=0; i<128; i++){
		src[i]=FFTin[i];
	}

	copy_in(FFTout, src, sizeof(fftwf_complex), 128, DFT_MIRROR);
	fftwf_execute(plan128F);
	for(i=0; i<128; i++){
		input[i]=input[i]/64.0;
	}
*/
/*	gain=10.0;
	//plan2048 = fftwf_plan_dft_1d(2048, FFTin,FFTout, FFTW_BACKWARD, FFTW_ESTIMATE);
	for(i=0; i<2048; i++){
		__real__ input[i]=gain*cos((3.1416/64.0)*(float)i);
		__imag__ input[i]=gain*sin((3.1416/64.0)*(float)i);
	}
*/
//	fftwf_execute(plan128F);
//	for(i=0; i<128; i++){
//		checkout[i]=FFTout[i];
//	}












	/////////////////////
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

	/////////////////////////TEST SSS
	//Select FFTsize
	FFTsize=128;
	phyLayerCellID=235;
	modinfo_msg("TxphyLayerCellID=%d\n", phyLayerCellID);
	//Generate SSS
/*	loadmtable (m0s, m1s);
	//setSSS(phyLayerCellID, SSSsf0, SSSsf5, m0s, m1s);
	genSSStime_seq(phyLayerCellID, FFTsize, SSSsubfr0time, SSSsubfr5time);
	detect_SSS (FFTsize, SSSsubfr0time, phyLayerCellID%3, m0s, m1s, &RxphyLayerCellID, &nofsubframe);
	//getCellId (SSSsf0, phyLayerCellID%3, m0s, m1s, &RxphyLayerCellID, &nofsubframe);
	printf("RxphyLayerCellID=%d, nofsubframe=%d\n", RxphyLayerCellID, nofsubframe);
*/



	//Generate SSS sequence to be send
	initSSStables(zz, ss, cc);
	genSSStime_seq(phyLayerCellID, FFTsize, SSSsubfr0time, SSSsubfr5time);
	//Normalize data
	normCvector(SSSsubfr0time, FFTsize);
	normCvector(SSSsubfr5time, FFTsize);


	////////////////////////////

	//Select FFTsize
	FFTsize=128;

	// Initialize vectors
	memset(AUXtime0, 0, sizeof(fftwf_complex)*(MAXDATASIZE));
	memset(AUXtime1, 0, sizeof(fftwf_complex)*(MAXDATASIZE));
	memset(noiseI, 0, sizeof(float)*(MAXDATASIZE));
	memset(noiseQ, 0, sizeof(float)*(MAXDATASIZE));

	//Generate PSS sequence to be send
	genPSStime_seq(1, FFTsize, AUXtime0, -1);

	//Add CP
	CPsize=CP_NOR_Slot1(FFTsize);
	modinfo_msg("CPsize=%d\n", CPsize);
	//Make space before
	for(i=0; i<FFTsize; i++){
		AUXtime0[FFTsize-1+CPsize-i]=AUXtime0[FFTsize-1-i];
		//printf("FFTsize-1+CPsize-i=%d FFTsize-1-i=%d\n", FFTsize-1+CPsize-i, FFTsize-1-i);
	}

	//Copy back to head
	for(i=0; i<CPsize; i++){
		AUXtime0[CPsize-1-i]=AUXtime0[FFTsize-1+CPsize-i];

	}

	//Normalize data
	normCvector(AUXtime0, FFTsize+CPsize);
	//Add noise
	EbNodBs=10.0;
	addnoise(EbNodBs, MAXDATASIZE, noiseI);
	addnoise(EbNodBs, MAXDATASIZE, noiseQ);
	//Generate output
	firstPSS=400;						//OFFSET
	modinfo_msg("firstPSS=%d\n", firstPSS);
	delaybetweenPSS = 10*SIZEOF_SLOT_CPNOR(FFTsize);	//CP normal, FFTsize=512
	modinfo_msg("delaybetweenPSS = %d\n", delaybetweenPSS);

	for(i=0; i<block_length; i++){
		__real__ AUXtime1[i] = noiseI[i];
		__imag__ AUXtime1[i] = noiseQ[i];
	}

	j=firstPSS;
	CPsize=CP_NOR_SlotN(FFTsize);
	diffCPs=CP_NOR_Slot1(FFTsize)-CP_NOR_SlotN(FFTsize);
	//ADD SSS SUBFRAME 0
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr0time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr0time[i-j];
	}
	//ADD PSS							//PSS0
	j=firstPSS+FFTsize;
	modinfo_msg("firstPSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	j=firstPSS+delaybetweenPSS;
	//ADD SSS SUBFRAME 5
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr5time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr5time[i-j];
	}
	//ADD PSS							//PSS1
	j=firstPSS+delaybetweenPSS+FFTsize;
	modinfo_msg("Second PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	//ADD SSS SUBFRAME 0
	j=firstPSS+2*delaybetweenPSS;
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr0time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr0time[i-j];
	}
	//ADD PSS							//PSS2
	j=firstPSS+2*delaybetweenPSS+FFTsize;
	modinfo_msg("Third PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	//ADD SSS SUBFRAME 5
	j=firstPSS+3*delaybetweenPSS;
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr5time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr5time[i-j];
	}
	//ADD PSS							//PSS3
	modinfo("TEST_GENERATOR EXTRA DELAY ADDED\n");
	j=firstPSS+3*delaybetweenPSS+FFTsize;
	modinfo_msg("Forth PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	//ADD SSS SUBFRAME 0
	j=firstPSS+4*delaybetweenPSS;
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr0time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr0time[i-j];
	}
	//ADD PSS							//PSS4
	j=firstPSS+4*delaybetweenPSS+FFTsize;
	modinfo_msg("Fith PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	//ADD SSS SUBFRAME 5
	j=firstPSS+5*delaybetweenPSS;
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr5time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr5time[i-j];
	}
	//ADD PSS							//PSS5
	j=firstPSS+5*delaybetweenPSS+FFTsize;
	modinfo_msg("Sixth PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	//ADD SSS SUBFRAME 0
	j=firstPSS+6*delaybetweenPSS;
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr0time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr0time[i-j];
	}
	//ADD PSS							//PSS6
	j=firstPSS+6*delaybetweenPSS+FFTsize;
	modinfo_msg("Seventh PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}
	//ADD SSS SUBFRAME 0
	j=firstPSS+7*delaybetweenPSS;
	for(i=j; i<j+FFTsize; i++){
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ SSSsubfr5time[i-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ SSSsubfr5time[i-j];
	}
	j=firstPSS+7*delaybetweenPSS+FFTsize;				//PSS7
	modinfo_msg("Eight PSS pointer=%d\n", j);
	for(i=j; i<j+FFTsize+CPsize; i++){	//CP 36
		__real__ AUXtime1[i] = __real__ AUXtime1[i]+__real__ AUXtime0[i+diffCPs-j];
		__imag__ AUXtime1[i] = __imag__ AUXtime1[i]+__imag__ AUXtime0[i+diffCPs-j];
	}

	//Quantify input data to 16 bits
	nofbitsDA=16;
	quantifyCvectorGO(AUXtime1, input, block_length, nofbitsDA);


	// HERE INDICATE THE LENGTH OF THE SIGNAL

/*	for(i=0; i<block_length; i++){
		input[i]=checkout[i];
	}
*/
	//printf("TEST_0\n");
	lengths[0] = block_length;
	modinfo("////////////////TEST GENERATE END\n");

	return 0;
}

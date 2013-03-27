/*
 * synch.c
 *
 *  Created on: 24/06/2012
 *      Author: xavier
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#define INCLUDE_DEFS_ONLY

#include "utils.h"
#include "syncSignals.h"
#include "dspfunct.h"
#include "PSSfilters.h"
#include "syncProcess.h"
#include "lte_synchG.h"

//Global variables
float LPPSS_128filter[128];
float LPPSS_256filter[128];
float LPPSS_512filter[128];
float LPPSS_1024filter[128];
float LPPSS_1536filter[128];
float LPPSS_2048filter[128];


/**@ingroup synchro()
 * This module find the maximum value of correlation.
 * First calculates correlation with a strong decimation factor, divides the correlation
 * sequence for finding local maximun and after ordering looks for the maximum without decimating.
 * \param numsamples: Samples from input
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * \coeff0, coeff1, coeff2: PSS time sequences.
 * \param *pMAXFinal: Pointer to structure containing the details of maximum of correlation found.
 * Return -1 i error, 1 i correlation max found and 0 if correlation not found.
 */
int synchroPSS(int numsamples, int FFTsize, _Complex float *in, _Complex float *out,\
		_Complex float *coeff0, _Complex float *coeff1, _Complex float *coeff2, p2MAX_t *pMAXFinal){

	int i, j;
	int decim, Nsync=64;
	int numblocks;
	static float threshold = CORR_THRESHOLD;
	_Complex float filteredIN[MAXINPUTLENGTH];
	_Complex float correl0[MAXINPUTLENGTH + CORRLENGTH];
	p2MAX_t p2MAXin[MAXNOFMAX];		//Non Ordered sequence of pointers to MAX among three sequences.
	p2MAX_t p2MAXout[MAXNOFMAX];	//Ordered sequence of pointers to MAX among three sequences.
	int initpoint, initcoeff, coefflength;
	_Complex float *coeff;

	if(FFTsize > CORRLENGTH){
		moderror_msg("Invalid FFTsize %d\n", FFTsize);
		return -1;
	}
	//Filter input to extract PSS
	if(FFTsize==128)PSS_LPrealfiltering(numsamples, in, filteredIN, LPPSS_128filter, 31);
	if(FFTsize==512)PSS_LPrealfiltering(numsamples, in, filteredIN, LPPSS_512filter, 33);

	/**Calculate decimated correlation for each sequence*/
	Nsync=32;	//NUmber of samples per FFT symbol
	decim = FFTsize / Nsync;
	//moddebug("FFTsize=%d, decim=%d\n", FFTsize, decim);
	// PSS Sequence 0
	vector_convolution(numsamples, decim, filteredIN, coeff0, FFTsize, out);
	findMAX(numsamples, FFTsize, decim, out, PSS_0, p2MAXin, &numblocks);
	// PSS Sequence 1
	vector_convolution(numsamples, decim, filteredIN, coeff1, FFTsize, out);
	findMAX(numsamples, FFTsize, decim, out, PSS_1, p2MAXin, &numblocks);
	// PSS Sequence 2
	vector_convolution(numsamples, decim, filteredIN, coeff2, FFTsize, out);
	findMAX(numsamples, FFTsize, decim, out, PSS_2, p2MAXin, &numblocks);
	numblocks*=3;
	orderMAXs(numblocks, p2MAXin, p2MAXout);

	decim=1;
	numsamples=FFTsize/2;	// Number of correlation samples to calculate
	for(i=0; i<numblocks; i++){
		memcpy(pMAXFinal, &p2MAXout[i], sizeof(p2MAX_t));
		//printf("TRY");
		//printf("%d ", i+1);
		//printf("p2MAXout[%d].seqNumber = %d\n", i, pMAXFinal->seqNumber);
		//printf("p2MAXout[%d].MAX = %3.3f\n", i, pMAXFinal->MAX);
		//printf("p2MAXout[%d].pMAX = %d\n", i, pMAXFinal->pMAX);
		initpoint=pMAXFinal->pMAX - FFTsize/2;
		coefflength=FFTsize;
		if(initpoint < 0)initpoint = 0;
		if(pMAXFinal->seqNumber == 0)coeff=coeff0;
		if(pMAXFinal->seqNumber == 1)coeff=coeff1;
		if(pMAXFinal->seqNumber == 2)coeff=coeff2;
		vector_convolution(numsamples, decim, in+initpoint, coeff, coefflength, out);
		if(findMAXFinal(numsamples, out, pMAXFinal, threshold) == 1){
			pMAXFinal->pMAX+=initpoint;
			moddebug("PSS_SEQ%d: pMAXFinal= %d, MAXfinal=%3.3f \n", pMAXFinal->seqNumber, pMAXFinal->pMAX, pMAXFinal->MAX);
			moddebug("average = %3.3f, max2average=%3.3f\n",
				pMAXFinal->average, pMAXFinal->max2average);
			moddebug("var=%3.3f, max2var=%3.3f \n",
						pMAXFinal->var, pMAXFinal->max2var);
			return 1;	//Correlation found
		}
	}
	pMAXFinal->MAX=0.; //Default Value
	pMAXFinal->pMAX=0;
	return 0;	//Correlation Not found
}


/**@ingroup initframe_alignment()
 * This module extract the relevant parameters for synchronization purposes. They are defined at
 * subframCtrll and returned by this variable. This function assumes the reception of a LTE subframe.
 * \param numsamples: Samples from input
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * \coeff0, coeff1, coeff2: PSS time sequences.
 * \param *subframCtrll: Pointer to structure containing the details of synchro status.
 * 	typedef struct subframe{
 *	int CPtype;
 *	int p2_subframe;	//point to 1st sample of subframe (including CPs)
 *	int nofsubframe;	//Current number of subframe in half LTE frame: 0 ..5
 *	int p2_OFDMPSS;		//Point to 1st sample of OFDM PSS symbol (including CP)
 *	int nofOFDMsymb;	//Current number of OFDM symb in half LTE frame: 0..NOFSYMBLTEFRAME/2
 *	int nofslot;		//Current number for slot in half LTE frame: 0..10
 *	int synchstate;		//Define the subframe detection state: INIT, TRACK
 *	int PSSseq;
 *	}subframectrl_t;
 *
 * Return -1 i error, 1 i correlation max found and 0 if correlation not found.
 */

int initframe_alignment(int numsamples, int FFTsize, _Complex float *in, _Complex float *correl,\
		_Complex float *coeff0, _Complex float *coeff1, _Complex float *coeff2,
		synchctrl_t *subframCtrl){

	int i,j,k;
	int corrdtect=0, pointer;	/** Point to the max value in correlation sequence*/
	p2MAX_t corrINFO;


	//Detect the first PSS
	memset(&corrINFO, 0, sizeof(p2MAX_t));
	if(subframCtrl->synchstate==SYNCH_INIT){
		moddebug("////////////////////////////////////SYNCH_INIT\n",0);
		moddebug("subframCtrl->nofsubframe=%d\n", subframCtrl->nofsubframe);
		corrdtect=synchroPSS(numsamples, FFTsize, in, correl, coeff0, coeff1, coeff2, &corrINFO);
		moddebug("SYNCHRO OUTPUT: corrdtect = %d\n",corrdtect);
		if(corrdtect == 0) {		//NO Correl Max found
			subframCtrl->synchstate=SYNCH_INIT;
			return 0;
		}
		if(corrdtect == -1){
			subframCtrl->synchstate=SYNCH_ERROR;
			moderror_msg("Synchronization error synchState=%d\n", subframCtrl->synchstate);
			return -1;
		}
		if(corrdtect == 1){
			subframCtrl->synchstate=SYNCH_TRACK;
			subframCtrl->p2_OFDMPSS = corrINFO.pMAX-FFTsize; //Pointer to the FFT symbol, after CP
			subframCtrl->PSSseq = corrINFO.seqNumber;
			subframCtrl->nofsubframe = 0;
			subframCtrl->nofslot = 0;
			//moddebug("p2_OFDMPSS=%d\n", subframCtrl->p2_OFDMPSS);
			return 1;
		}
		moderror_msg("Invalid value for corrdtect=%d \n", corrdtect);
		return 0;
	}
	//if(subframCtrl->synchstate==SYNCH_TRACK && (subframCtrl->nofsubframe == 0 || subframCtrl->nofsubframe == 5)){
	if(subframCtrl->synchstate==SYNCH_TRACK){
		if(subframCtrl->nofsubframe != 0 && subframCtrl->nofsubframe != 5)return 0; //Do nothing in not synchro subframes
		moddebug("subframCtrl->nofsubframe=%d\n", subframCtrl->nofsubframe);

		moddebug("////////////////////////////////////SYNCH_TRACK\n",0);
		corrdtect=synchroPSS(numsamples, FFTsize, in/*+pointer*/, correl, coeff0, coeff1, coeff2, &corrINFO);
		//Option 2: Reduce the correlation search
		//numsamples is assumed to be 1 subframe length
		//in = (_Complex float *) (in + numsamples - 3*FFTsize);
		//numsamples = 3*FFTsize;
		//corrdtect=synchroPSS(numsamples, FFTsize, in, correl, coeff0, coeff1, coeff2, &corrINFO);
		moddebug("SYNCHRO OUTPUT: corrdtect = %d\n",corrdtect);
		if(corrdtect == 0) {		//NO Correl Max found
			subframCtrl->synchstate=SYNCH_INIT;
			return 0;
		}
		if(corrdtect == -1){
			subframCtrl->synchstate=SYNCH_ERROR;
			moderror_msg("Synchronization error synchState=%d\n", subframCtrl->synchstate);
			return -1;
		}
		if(corrdtect == 1){
			subframCtrl->synchstate=SYNCH_TRACK;
			subframCtrl->p2_OFDMPSS = /*pointer+*/corrINFO.pMAX-FFTsize;
			subframCtrl->nofsubframe = 0;
			subframCtrl->nofslot = 0;
			moddebug("p2_OFDMPSS=%d\n", subframCtrl->p2_OFDMPSS);
			return 1;
		}
		moderror_msg("Invalid value for corrdtect=%d \n", corrdtect);
		return 0;
	}
	return 0;
}


/**@ingroup detect_CP()
 * Detects the current CP: Normal or Extended.
 * Calculates R, the max of correlation of CP sequence with the end of OFDM symbol and the CP.
 * Calculates P, the max of correlation of CP lenght of the end of OFDM symbol.
 * Divides R/P for both the normal and extended CP. The higher the winner.
 * \param fftpointer: defines the first position of the OFDM symbol in the input_data sequence.
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * Return 0 for no value detected, 1 for CP normal; 2 for extended, -1 for error.
 */

int detect_CP(int fftpointer, int FFTsize, _Complex float *in){

	int i;
	int nofsamples, decimate, CP_size;
	float RCPnorm, RCPext, PCPnorm, PCPext, MCPnorm, MCPext;
	_Complex float stateCPdet[160];
	_Complex float  *pdata;		//Data pointer to the last 40 samples in the FFT symbol
	_Complex float  *pdataF;	//Data pointer to the last sample in the FFT symbol
	_Complex float  *pCP;		//Data pointer to the CP expected position
	//_Complex float coeffCP[160];
	_Complex float aux;

	//CP Normal
	CP_size=CP_NOR_SlotN(FFTsize);
	nofsamples=CP_size;
	decimate=1;
	pCP = (_Complex float *) (in+(fftpointer-CP_size));
	pdata = (_Complex float *) (in+(fftpointer-CP_size)+FFTsize);

	//Max of corr between CP and final of OFDM symbol
	RCPnorm=0.;
	for(i=0; i<CP_size; i++){
		aux = *(pCP+i);
		__imag__ aux = -(__imag__ aux);
		RCPnorm += *(pdata+i)*aux;
		//printf("PCPnorm=%3.3f\n", PCPnorm);
	}
	//Calculate Maximum of autocorrelation of final part of OFDM symbol
	PCPnorm=0.;
	for(i=0; i<CP_size; i++){
		aux = *(pdata+i);
		__imag__ aux = -(__imag__ aux);
		PCPnorm += *(pdata+i)*aux;
	}
	MCPnorm = RCPnorm/PCPnorm;

	//CP Extended
	CP_size=CP_EXT15K(FFTsize);
	nofsamples=CP_size;
	decimate=1;
	pCP = (_Complex float *) (in+(fftpointer-CP_size));
	pdata = (_Complex float *) (in+(fftpointer-CP_size)+FFTsize);
	//Calculate MAX of correlation CP & end of OFDM symbol
	RCPext=0.;
	for(i=0; i<CP_size; i++){
		aux = *(pCP+i);
		__imag__ aux = -(__imag__ aux);
		RCPext += *(pdata+i)*aux;
	}
	PCPext=0.;
	for(i=0; i<CP_size; i++){
		aux = *(pdata+i);
		__imag__ aux = -(__imag__ aux);
		PCPext += *(pdata+i)*aux;
	}
	MCPext = RCPext/PCPext;

	moddebug("RCPnorm=%3.1f, PCPnorm=%3.1f, MCPnorm=%3.3f,\n", RCPnorm, PCPnorm, MCPnorm);
	moddebug("RCPext=%3.1f, PCPext=%3.1f, MCPext=%3.3f,\n", RCPext, PCPext, MCPext);

	if(MCPnorm > MCPext)return CP_NOR; 	//CP Normal
	if(MCPnorm < MCPext)return CP_EXT1;	//CP Extended
	if(MCPnorm == MCPext)return CP_UNKNOWN;
}




/**@ingroup findMAXFinal()
 * Find a Max of correlation with a rate Max/variance > threshold.
 * \param numsamples:
 * \param corr: Correlation sequence.
 * \param *pMAX2: Pointer to the list of maximums and their parameters.
 * \param threshold:decission treshold for MAX/Var parameter.
 * Return 1 if success, 0 otherwise.
 */

int findMAXFinal(int numsamples, _Complex float *corr, p2MAX_t *pMAXe, float threshold){

	int i;

	//Find Max total
	pMAXe->MAX=0.;
	for(i=0; i<numsamples; i++){
		if(pMAXe->MAX < __real__ corr[i]){
			pMAXe->MAX = __real__ corr[i];
			pMAXe->pMAX = i;
		}
		pMAXe->average += __real__ corr[i];
		pMAXe->var += fabsf(__real__ corr[i]);
	}
	pMAXe->average /= numsamples;
	pMAXe->var /= numsamples;
	pMAXe->max2average=pMAXe->MAX/pMAXe->average;
	pMAXe->max2var=pMAXe->MAX/pMAXe->var;
//	printf("pMAX = %d, MAX = %3.3f, average = %3.3f, MAX/average=%3.3f, var = %3.3f MAX/var=%3.3f\n",
//			pMAXe->pMAX, pMAXe->MAX, pMAXe->average, pMAXe->max2average, pMAXe->var, pMAXe->max2var);
	if(pMAXe->max2var > threshold){
		//moddebug("pMAX = %d, MAX = %3.3f, average = %3.3f, MAX/average = %3.3f, var = %3.3f MAX/var = %3.3f \n",
		//	pMAXe->pMAX, pMAXe->MAX, pMAXe->average, pMAXe->max2average, pMAXe->var, pMAXe->max2var);
		return 1;
	}
	else return 0;
}



/**@ingroup findMAX()
 * Find the list of Max in the correlation sequence.
 * \param numsamples:
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * \param decimate: set a decimation factor.
 * \param corr: Correlation sequence.
 * \param numPSSseq: Number of PSS sequence results to evaluate.
 * \param *pMAX2: Output. Pointer to the list of maximums and their parameters.
 * \param *numblocks:Output. Number of MAX in the generated list.
 */

void findMAX(int numsamples, int FFTsize, int decimate, _Complex float *corr, int numPSSseq, p2MAX_t *p2MAX, int *numblocks){

	int i, j, k, pmax;
	int window;
	float max, max2average;

	window = (FFTsize/4)*decimate;
	numsamples=numsamples/decimate;
	if(numsamples%window == 0){
		*numblocks=(numsamples/window);
	}
	else {
		*numblocks=numsamples/window + 1;
	}
	pmax=0;
	max=0.;
	max2average=0.;
	k=*numblocks*numPSSseq;
	for(j=0; j<*numblocks; j++){
		p2MAX[k].pMAX=0;
		p2MAX[k].MAX=0.;
		p2MAX[k].seqNumber = numPSSseq;
		p2MAX[k].average = 0.;
		p2MAX[k].max2average = 0.;
		p2MAX[k].var = 0.;
		p2MAX[k].max2var = 0.;
		for(i=0; i<window; i++){
			if(p2MAX[k].MAX < __real__ corr[i+j*window]){
				p2MAX[k].MAX = __real__ corr[i+j*window];
				p2MAX[k].pMAX = (i+j*window)*decimate;
			}
			p2MAX[k].average += __real__ corr[i+j*window];
			p2MAX[k].var += fabsf(__real__ corr[i+j*window]);
		}
		//p2MAX[k].average -= p2MAX[k].MAX;
		p2MAX[k].average /= window;
		p2MAX[k].max2average = p2MAX[k].MAX / p2MAX[k].average;
		//p2MAX[k].var -= p2MAX[k].MAX;
		p2MAX[k].var /= window;
		p2MAX[k].max2var = p2MAX[k].MAX / p2MAX[k].var;


		//moddebug("pMAX[%d]=%d MAX[]=%3.3f average[]=%3.3f max2average=%3.3f var[]=%3.3f max2var=%3.3f\n",
		//	k, p2MAX[k].pMAX, p2MAX[k].MAX, p2MAX[k].average, p2MAX[k].max2average, p2MAX[k].var, p2MAX[k].max2var);

		k++;
	}
}

//Return -1 if error

int orderMAXs(int nofMAXs, p2MAX_t *pMAXin, p2MAX_t *pMAXout){
	int i, j, k, pmax;
	p2MAX_t pMAXaux[MAXNOFMAX];

	if(nofMAXs>MAXNOFMAX){
		return -1;
	}
	memset(pMAXout, 0, sizeof(p2MAX_t)*MAXNOFMAX);
	k=0;
	for(k=0; k<nofMAXs; k++){
		for(i=0; i<nofMAXs; i++){
			if(pMAXout[k].MAX < pMAXin[i].MAX){
				pMAXout[k].MAX=pMAXin[i].MAX;
				pMAXout[k].average=pMAXin[i].average;
				pMAXout[k].max2average=pMAXin[i].max2average;
				pMAXout[k].pMAX=pMAXin[i].pMAX;
				pMAXout[k].seqNumber=pMAXin[i].seqNumber;
				pmax=i;
			}
		}
		pMAXin[pmax].MAX = 0.;
	}
}



int PSS_LPrealfiltering(int numsamples, _Complex float *in, _Complex float *out,
		float *LPfilter, int filterlength){

	int i;

	float *pstateI, stateI[MAXFILTERLENGHT];
	float *pstateQ, stateQ[MAXFILTERLENGHT];

	memset(stateI, 0, sizeof(float)*MAXFILTERLENGHT);
	memset(stateQ, 0, sizeof(float)*MAXFILTERLENGHT);
	pstateI = stateI;
	pstateQ = stateQ;

	for(i=0; i<numsamples; i++){
		__real__  out[i] = fullconv(__real__ *(in+i), &pstateI, stateI, filterlength, 1, LPfilter);
		__imag__  out[i] = fullconv(__imag__ *(in+i), &pstateQ, stateQ, filterlength, 1, LPfilter);
	}
	return 0;
}


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
#include "lte_synchF.h"
#include "utils.h"
#include "syncSignals.h"
#include "dspfunct.h"

//Global variables

/**@ingroup vector_convolution
 */
int vector_convolution(int numsamples, int decimate, _Complex float *in,\
			_Complex float *coeffi, int seqlength, _Complex float *corr){

	int i, j;
	int length=seqlength/decimate;
	int num=numsamples/decimate;
	_Complex float state[MAXCORRWINDOW];
	_Complex float *pstate;

	if(length > MAXCORRWINDOW){
		modinfo_msg("filter length too long%d\n",length);
		return 0;
	}
	memset(state, 0, sizeof(_Complex float)*MAXCORRWINDOW);
	pstate = state;
	for(i=0; i<num; i++){
		corr[i] = complex_conv(*(in+i*decimate), &pstate, state, coeffi, length, decimate);
	}
	return(1);	/** return 1 if OK */
}

/**@ingroup vector_correlation
 */
float vector_correlation(int length_in1, int decimate, _Complex float *in1,\
			_Complex float *in2, int length_in2, _Complex float *corr){

	int i, j;
	int length=length_in2/decimate;
	int num=length_in1/decimate;
	_Complex float state[MAXCORRWINDOW], coeff[MAXCORRWINDOW];
	_Complex float *pstate;
	float CORRvalue=0.;

	if(length_in2 > MAXCORRWINDOW){
		modinfo_msg("filter length too long%d\n",length);
		return 0;
	}
	memset(state, 0, sizeof(_Complex float)*MAXCORRWINDOW);
	pstate = state;
	for(i=0; i<num; i++){
		corr[i] = complex_correlation(*(in1+i*decimate), &pstate, state, in2, length, decimate);
		CORRvalue += __real__ corr[i];
	}
	return(CORRvalue);
}


/** Normalize the complex vector according the maxval value
 * in and out in same vector */
void normCvector2Maxvalue(_Complex float *inout, int length, float maxval){
	int i;

	for(i=0; i<length; i++){
		inout[i]=inout[i]/maxval;
	}
}


/** Normalize the complex vector to the max value
 * in and out in same vector */
void normCvector(_Complex float *inout, int length){
	int i;
	float maxval;

	maxval = 0;
	for(i=0; i<length; i++){
		if(maxval < __real__ inout[i]){
			maxval = __real__ inout[i];
		}
		if(maxval < __imag__ inout[i]){
			maxval = __imag__ inout[i];
		}
	}
	for(i=0; i<length; i++){
		inout[i]=inout[i]/maxval;
	}
}


/** Normalize the complex vector to the max value
 * in and out in different vector
 * */
void normCvectorGO(_Complex float *in, _Complex float *out, int length){
	int i;
	float maxval;

	maxval = 0;
	for(i=0; i<length; i++){
		if(maxval < __real__ in[i]){
			maxval = __real__ in[i];
		}
		if(maxval < __imag__ in[i]){
			maxval = __imag__ in[i];
		}
	}
	for(i=0; i<length; i++){
		out[i]=in[i]/maxval;
	}
}

/** Quantify the complex array in to integers of n bits
 */
void quantifyCvectorGO(_Complex float *in, _Complex float *out, int length, int n){
	int i;
	float maxval;

	maxval = 0;
	for(i=0; i<length; i++){
		if(maxval < __real__ in[i]){
			maxval = __real__ in[i];
		}
		if(maxval < __imag__ in[i]){
			maxval = __imag__ in[i];
		}
	}
	printf("(1<<(n-1))=%d\n", (1<<(n-1)));
	for(i=0; i<length; i++){
		__real__ out[i]=(float)((int)(((__real__ in[i])/maxval)*(1<<(n-1))));
		__imag__ out[i]=(float)((int)(((__imag__ in[i])/maxval)*(1<<(n-1))));
	}
}

/* float convolution step
 */

float fullconv(float input, float **pbuffrec, float *buffrec, int length, int delmat, float *coef)
{
 int i;
 float salida = 0.;

 **pbuffrec = input;
 for(i=0; i<length; i++)
 {
  salida += **pbuffrec * coef[i*delmat];
  if(i < length-1) (*pbuffrec)++;
  if(*pbuffrec == buffrec + length) *pbuffrec = buffrec;
 }
 return salida;
}

/** Complex Convolution step
 */
_Complex float complex_conv(_Complex float input, _Complex float **pbuffrec,
		_Complex float *buffrec,_Complex float *coef,
		int length, int decimate){

	int i;
	_Complex float outval;

	__real__ outval = 0.;
	__imag__ outval = 0.;

	**pbuffrec = input;

	for(i=0; i<length; i++){
		outval += **pbuffrec *  coef[i*decimate];
		if(i < length-1) (*pbuffrec)++;
		if(*pbuffrec >= buffrec + length) *pbuffrec = buffrec;
	}
 return outval;
}

/* Complex correlation step
 */
_Complex float complex_correlation(_Complex float input1, _Complex float **pbuffrec,
		_Complex float *buffrec,_Complex float *input2,
		int length, int decimate){

	int i;
	_Complex float outval, aux;

	__real__ outval = 0.;
	__imag__ outval = 0.;

	**pbuffrec = input1;
	for(i=0; i<length; i++){
		aux = input2[i*decimate];	//To avoid modifying data
		//Conjugate
		__imag__ aux = -(__imag__ aux);
		outval += **pbuffrec * aux;
		if(i < length-1) (*pbuffrec)++;
		if(*pbuffrec >= buffrec + length) *pbuffrec = buffrec;
	}
 return outval;
}

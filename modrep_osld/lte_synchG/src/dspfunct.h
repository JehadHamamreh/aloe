

#ifndef _DSPFUNCT_H
#define _DSPFUNCT_H

#define MAXCORRWINDOW	2048
#define MAXFILTERLENGHT	128

int vector_convolution(int numsamples, int decimate, _Complex float *in,\
			_Complex float *coeffi, int seqlength, _Complex float *corr);
float vector_correlation(int length_in1, int decimate, _Complex float *in1,\
			_Complex float *in2, int length_in2, _Complex float *corr);

void normCvector2Maxvalue(_Complex float *inout, int length, float maxval);
void normCvectorCvector(_Complex float *inout, int length);
void normCvectorGO(_Complex float *in, _Complex float *out, int length);
void quantifyCvectorGO(_Complex float *in, _Complex float *out, int length, int n);
//int create_fft_plan();
_Complex float complex_conv(_Complex float input, _Complex float **pbuffrec, _Complex float *buffrec,\
							_Complex float *coef, int length, int decimate);
_Complex float complex_correlation(_Complex float input1, _Complex float **pbuffrec,
		_Complex float *buffrec,_Complex float *input2, int length, int decimate);
_Complex float _Cconv(_Complex float input, _Complex float *buffrec,\
							_Complex float *coef, int length, int delmat);
float fullconv(float input, float **pbuffrec, float *buffrec, int length, int delmat, float *coef);
int init_ifft_LTE(int FFTsize, _Complex float *in, _Complex float *out);
int ifft_LTE(int FFTsize, _Complex float *in, _Complex float *out);

#endif

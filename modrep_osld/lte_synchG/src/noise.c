/* BPSK BINARY SYMMETRIC CHANNEL SIMULATOR                          */
/* Copyright (c) 1999, Spectrum Applications, Derwood, MD, USA      */
/* All rights reserved                                              */
/* Version 2.0 Last Modified 1999.02.17                             */


//#include <alloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include "noise.h"

#define PI 3.14159

//#define RAND_MAX 	2147483647
#define LRAND_MAX 	0x7FFFFFFFU

float gngauss(float mean, float sigma);
void addnoise(float es_ovr_n0, long channel_len, float *in_array);

void addnoise(float es_ovr_n0, long channel_len, float *in_array) {

    long t;
    float mean, es, sn_ratio, sigma, signal;


    /* given the desired Es/No (for BPSK, = Eb/No - 3 dB), calculate the
    standard deviation of the additive white gaussian noise (AWGN). The
    standard deviation of the AWGN will be used to generate Gaussian random
    variables simulating the noise that is added to the signal. */

    mean = 0;
    es = 1;
    sn_ratio = (float) pow(10, ( es_ovr_n0 / 10) );
    sigma =  (float) sqrt (es / (2*sn_ratio ) );

    for (t = 0; t < channel_len; t++) {
        *( in_array + t ) += gngauss(mean, sigma);
    }
}



float gngauss(float mean, float sigma) {

    /* This uses the fact that a Rayleigh-distributed random variable R, with
    the probability distribution F(R) = 0 if R < 0 and F(R) =
    1 - exp(-R^2/2*sigma^2) if R >= 0, is related to a pair of Gaussian
    variables C and D through the transformation C = R * cos(theta) and
    D = R * sin(theta), where theta is a uniformly distributed variable
    in the interval (0, 2*pi()). From Contemporary Communication Systems
    USING MATLAB(R), by John G. Proakis and Masoud Salehi, published by
    PWS Publishing Company, 1998, pp 49-50. This is a pretty good book. */


    double u, r;            /* uniform and Rayleigh random variables */


    /* generate a uniformly distributed random number u between 0 and 1 - 1E-6*/
    //u = (double)_lrand() / LRAND_MAX;
     u = (double)rand() / LRAND_MAX;
    if (u == 1.0) u = 0.999999999;


    /* generate a Rayleigh-distributed random number r using u */
    r = sigma * sqrt( 2.0 * log( 1.0 / (1.0 - u) ) );

    /* generate another uniformly-distributed random number u as before*/
    //u = (double)_lrand() / LRAND_MAX;
    u = (double)rand() / LRAND_MAX;
    if (u == 1.0) u = 0.999999999;

    /* generate and return a Gaussian-distributed random number using r and u */


    return( (float) ( mean + r * cos(2 * PI * u) ) );
}

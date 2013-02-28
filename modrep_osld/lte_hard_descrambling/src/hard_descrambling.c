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
#include <params.h>
#include <skeleton.h>
#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "hard_descrambling.h"

unsigned x1[MAX_x];
unsigned x2[MAX_x][10];


/**
 * @ingroup Transforms chars to unsiged integers
 * Each char contains a single bit, i.e., a signle 0 or 1.
 * Each integer contains 32 bits.
 *
 * \param input Pointer to input sequence (1D array of chars)
 * \param output Pointer t output sequence (1D array of unsighed integers)
 * \param N Number of input samples (# chars)
  */
inline void char2int(char *input, unsigned *output, int N)
{
	int i, j, s;
	int bits_per_int = 32;

	j = 1;
	s = 0;
	for (i=0; i<N/bits_per_int+1; i++) {
		output[i] = 0;
	}
	for (i=0; i<N; i++) {	/* input bits */
		if (i==j*bits_per_int) {
			s = 0;
			j++;
		}
		output[j-1]+=input[i]<<s;
		s++;
	}
}

/**
 * @ingroup Transforms integers to chars.
 * Each integer contains 32 bits.
 * Each char contains a single bit, i.e., a signle 0 or 1.
 *
 * \param input Pointer to input sequence (1D array of unsigned integers)
 * \param output Pointer t output sequence (1D array of chars)
 * \param N Number of input samples (# chars)
 * \param rem_bits Number of remaining chars (bits) after integer division by 32
  */
inline void int2char(unsigned *input, char *output, int N, int rem_bits)
{
	int i, j;
	int bits_per_int = 32;
	int K;

	K = N/bits_per_int;	/* integer division */
	for (i=0; i<K; i++) {
		for (j=0; j<bits_per_int; j++) {
			output[i*bits_per_int+j] = (char)((input[i]>>j)&1);

		}
	}
	for (i=0; i<rem_bits; i++) {	/* remaining bits */
		output[K*bits_per_int+i] = (char)((input[K]>>i)&1);
	}
}

/**
 * @ingroup First m-sequence {x1}
 * Auxiliary function for generating the first m-sequence needed for
 * generating the scrambling sequence c for all 10 subframes.
 * x1 is independent of the subframe index.
 */
inline void compute_x1(void)
{
	int i, j, s, d;

	/* initialize first 32 bits of {x1} */
	x1[0] = 1; 	/* first 31 bits: 0..30 */
	x1[0] += 1<<31; /* bit position 31 is a '1' */
	/* compute remaining bits of {x1} */
	j = 1;
	s = 0;
	d = 0;
	x1[j] = 0;
	for (i=0; i<(MAX_x-1)*32; i++) {
		if (i == j*32) {
			j++;
			s = 0;
			d = 0;
			x1[j] = 0;
		}
		if (s<28) {
			x1[j] += ((((x1[j-1]>>(s+4))&1) ^ ((x1[j-1]>>(s+1))&1)))<<s;
		} else if (s<31) {
			x1[j] += ((((x1[j]>>d)&1) ^ ((x1[j-1]>>(s+1))&1)))<<s;
			d++;
		} else {
			x1[j] += ((((x1[j]>>3)&1) ^ (x1[j]&1)))<<s;
		}
		s++;
	}
}

/**
 * @ingroup Second m-sequence {x2}
 * Auxiliary function for generating the second m-sequence needed for
 * generating the scrambling sequence c for all 10 subframes.
 * x2 is a function of the subframe index.
 *
 * \param c_init Pointer to inialization values of x2 for each subframe
 */
inline void compute_x2(unsigned *c_init)
{
	int i, j, s, n;

	/* initialize first 32 bits of {x2} */
	for (n=0; n<10; n++) {
		x2[0][n] = c_init[n]; /* first 31 bits: 0..30 */
		/* initialize bit position 31: */
		x2[0][n] += ((((x2[0][n]>>3)&1)
			+ ((x2[0][n]>>2)&1)
			+ ((x2[0][n]>>1)&1)
			+ (x2[0][n]&1))%2)<<31;
	}

	/* compute remaining bits of {x2} */
	for (n=0; n<10; n++) {
		j = 1;
		s = 0;
		x2[j][n] = 0;
		for (i=0; i<(MAX_x-1)*32; i++) {
			if (i == j*32) {
				j++;
				s = 0;
				x2[j][n] = 0;
			}
			if (s<28) {
				x2[j][n] += ((((x2[j-1][n]>>(s+4))&1)
					+ ((x2[j-1][n]>>(s+3))&1)
					+ ((x2[j-1][n]>>(s+2))&1)
					+ ((x2[j-1][n]>>(s+1))&1))%2)<<s;
			} else if (s==28) {
				x2[j][n] += (((x2[j][n]&1)
					+ ((x2[j-1][n]>>(s+3))&1)
					+ ((x2[j-1][n]>>(s+2))&1)
					+ ((x2[j-1][n]>>(s+1))&1))%2)<<s;
			} else if (s==29) {
				x2[j][n] += ((((x2[j][n]>>1)&1)
					+ (x2[j][n]&1)
					+ ((x2[j-1][n]>>(s+2))&1)
					+ ((x2[j-1][n]>>(s+1))&1))%2)<<s;
			} else if (s==30) {
				x2[j][n] += (((((x2[j][n])>>2)&1)
					+ ((x2[j][n]>>1)&1)
					+ (x2[j][n]&1)
					+ ((x2[j-1][n]>>(s+1))&1))%2)<<s;
			} else {
				x2[j][n] += (((((x2[j][n])>>3)&1)
					+ ((x2[j][n]>>2)&1)
					+ ((x2[j][n]>>1)&1)
					+ (x2[j][n]&1))%2)<<s;
			}
			s++;
		}
	}
}

/**
 * @ingroup x2-sequence intialization
 * Initialize second m-seqeunce {x2} for generating the scrambling sequence {c}.
 * For the maximum input size (max number of input samples) it generates all
 * bits of {x2} for all 10 subframes.
 *
 * \param c_init Pointer to initilization polynomial (first 31 bits)
 * \param params Structure containing the scrambling sequence generation parameters
 */
inline void x2init(unsigned *c_init, struct scrambling_params params)
{
	int n;	/* subframe index */
	int N_cell;

	N_cell = 3*params.cell_gr + params.cell_sec;

	if (params.channel == PDSCH) { /* also PUSCH */
		for (n=0; n<10; n++) {
			c_init[n] = (params.nrnti<<14)
				+ (params.q<<13)
				+ (n<<9)
				+ N_cell;
		}
	} else if (params.channel == PCFICH) {
		for (n=0; n<10; n++) {
			c_init[n] = (((n+1) * (2*N_cell+1))<<9) + N_cell;
		}
	} else if (params.channel == PDCCH) {
		for (n=0; n<10; n++) {
			c_init[n] = (n<<9) + N_cell;
		}
	} else if (params.channel == PBCH) {
		/* Caution: The scrambling sequence generator for the PBCH is
		 * not initialized on subframe basis */
		for (n=0; n<10; n++) {
			c_init[n] = N_cell;
		}
	} else if (params.channel == PMCH) {
		for (n=0; n<10; n++) {
			c_init[n] = (n<<9) + params.nMBSFN;
		}
	} else if (params.channel == PUCCH) {
		for (n=0; n<10; n++) {
			c_init[n] = ((n+1) * (2*N_cell+1)<<16) + params.nrnti;
		}
	}
}

/**
 * @ingroup LTE Scrambling Sequence generator
 * Generates the scrambling sequence based on the 3GPP specifications.
 * For the maximum input size (max number of input samples) it generates
 * all bits of the scrambling sequence for all 10 subframes.
 *
 * \param c Scrambling sequences for all 10 subframes and maximum input sample length
 * \param params Parameters that initialize the scrambling sequeznce generator
 */
inline void sequence_generation(unsigned (*c)[10], struct scrambling_params params)
{
	int i, n;
	unsigned c_init[10];

	/* compute m-sequence {x1} */
	compute_x1();

	/* compute m-sequence {x2} */
	x2init(c_init, params);
	compute_x2(c_init);

	/* compute scrambling sequence {c} */
	for (n=0; n<10; n++) {
		for (i=0; i<MAX_c; i++) {
			c[i][n] = x1[i+(params.Nc/32)] ^ x2[i+(params.Nc/32)][n];
		}
	}
}

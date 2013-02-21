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
#include "scrambling.h"

unsigned int x1[MAX_x];
unsigned int x2[MAX_x][10];

/**
 * @ingroup Transforms chars to unsiged integers
 * Each char contains a single bit, i.e., a signle 0 or 1.
 * Each integer contains 32 bits.
 *
 * \param input Pointer to input sequence (1D array of chars)
 * \param output Pointer t output sequence (1D array of unsighed integers)
 * \param N Number of input samples (# chars)
  */
inline void char2int(char *input, unsigned int *output, int N)
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
inline void int2char(unsigned int *input, char *output, int N, int rem_bits)
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
void compute_x1(void)
{
	int i, j, s, d;

	/* initialize first values {x1} */
	x1[0] = 1; /* initialize x1 (first 31 bits: 0..30)*/
	x1[0] += 1<<31; /* bit 31 is a '1' */
	/* compute remaining values of {x1} */
	j = 1;
	s = 0;
	d = 0;
	x1[j] = 0;
	for (i=0; i<MAX_x*32; i++) {
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
void compute_x2(unsigned int *c_init)
{
	int i, j, s, n;

	/* initialize first values of {x2} */
	for (n=0; n<10; n++) {
		x2[0][n] = c_init[n]; /* initialize x2 (first 31 bits: 0..30)*/
		x2[0][n] += ((((x2[0][n]>>3)&1)
			+ ((x2[0][n]>>2)&1)
			+ ((x2[0][n]>>1)&1)
			+ (x2[0][n]&1))%2)<<31;
	}

	/* compute remaining values of {x2} */
	for (n=0; n<10; n++) {
		j = 1;
		s = 0;
		x2[j][n] = 0;
		for (i=0; i<MAX_x*32; i++) {
			if (j<1 || j > MAX_x-1) {
				printf("j=%d\n",j);
			}
			if (n<0 || n>9) {
				printf("n=%d\n",n);
			}
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
 * @ingroup LTE Scrambling Sequence generator
 * Generates the scrambling sequence based on the 3GPP specifications.
 * For the maximum input size (max number of input samples) it generates
 * all bits of the scrambling sequence for all 10 subframes.
 *
 * \param c Scrambling sequences for all 10 subframes and maximum input sample length
 * \param params Parameters that initialize the scrambling sequeznce generator
 */
void sequence_generation(unsigned (*c)[10], struct scrambling_params params)
{
	int i, n;
	int N_cell;
	unsigned int c_init[10];

	/* compute the two m-sequences {x1} and {x2} */
	compute_x1();

	/* initialize x2 */
	N_cell = 3*params.cell_gr + params.cell_sec;
	for (n=0; n<10; n++) {
		c_init[n] = (params.nrnti<<14)/**16384*/
			+ (params.q<<13)/**8192*/
			+ (n<<9)/**512*/
			+ N_cell;
	}

	compute_x2(c_init);

	/* compute the scrambling sequence {c} */
	for (n=0; n<10; n++) {
		for (i=0; i<MAX_c; i++) {
			c[i][n] = x1[i+(params.Nc/32)] ^ x2[i+(params.Nc/32)][n];
		}
	}
}

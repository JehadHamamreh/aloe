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
#include <stdlib.h>
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>
#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "scrambling.h"

unsigned x1[MAX_x];
unsigned x2[NOF_SUBFRAMES][MAX_x];
unsigned input_ints[MAX_c], out_ints[MAX_c]; /* intermediate results */

//extern int pbch;

/**
 * @ingroup Identify placeholder (x) and repetition (y) bits
 * PUSCH
 *
 * \param in Input bit sequence (1D array of chars)
 * \param M Number of input samples
 * \param uparams Pointer to UL parameters structure (ul_params)
  */
inline void identify_xy(char *in, int M, struct ul_params *uparams) {

	int i;

	uparams->x_size = 0;
	uparams->y_size = 0;
	for (i=0; i<M; i++) {
		if (in[i] == 'x') {
			uparams->x[uparams->x_size] = i;
			uparams->x_size++;
			in[i] = 0;	/* set to 0 or 1, doesn't matter */
		}
		if (in[i] == 'y') {
			uparams->y[uparams->y_size] = i;
			uparams->y_size++;
			in[i] = 0;	/* set to 0 or 1, doesn't matter */
		}
	}
}

/**
 * @ingroup Set values of placeholder (x) and repetition (y) bits
 * PUSCH
 *
 * \param out Output bit sequence (1D array of chars)
 * \param uparams UL parameters structure (ul_params)
  */
inline void set_xy(char *out, struct ul_params uparams) {

	int i;

	for (i=0; i<uparams.x_size; i++) {
		/* ACK/NACK or Rank Indication placeholder bits */
		out[uparams.x[i]] = 0x1;
	}
	for (i=0; i<uparams.y_size; i++) {
		/* ACK/NACK or Rank Indication repetition placeholder bits */
		out[uparams.y[i]] = out[uparams.y[i]-1];
	}
}


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

	j = 1;
	s = 0;
	for (i=0; i<N/BIT_PER_INT+1; i++) {
		output[i] = 0;
	}
	for (i=0; i<N; i++) {	/* input bits */
		if (i==j*BIT_PER_INT) {
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
	for (n=0; n<NOF_SUBFRAMES; n++) {
		x2[n][0] = c_init[n]; /* first 31 bits: 0..30 */
		/* initialize bit position 31: */
		x2[n][0] += ((((x2[n][0]>>3)&1)
			+ ((x2[n][0]>>2)&1)
			+ ((x2[n][0]>>1)&1)
			+ (x2[n][0]&1))%2)<<31;
	}

	/* compute remaining bits of {x2} */
	for (n=0; n<NOF_SUBFRAMES; n++) {
		j = 1;
		s = 0;
		x2[n][j] = 0;
		for (i=0; i<(MAX_x-1)*32; i++) {
			if (i == j*32) {
				j++;
				s = 0;
				x2[n][j] = 0;
			}
			if (s<28) {
				x2[n][j] += ((((x2[n][j-1]>>(s+4))&1)
					+ ((x2[n][j-1]>>(s+3))&1)
					+ ((x2[n][j-1]>>(s+2))&1)
					+ ((x2[n][j-1]>>(s+1))&1))%2)<<s;
			} else if (s==28) {
				x2[n][j] += (((x2[n][j]&1)
					+ ((x2[n][j-1]>>(s+3))&1)
					+ ((x2[n][j-1]>>(s+2))&1)
					+ ((x2[n][j-1]>>(s+1))&1))%2)<<s;
			} else if (s==29) {
				x2[n][j] += ((((x2[n][j]>>1)&1)
					+ (x2[n][j]&1)
					+ ((x2[n][j-1]>>(s+2))&1)
					+ ((x2[n][j-1]>>(s+1))&1))%2)<<s;
			} else if (s==30) {
				x2[n][j] += (((((x2[n][j])>>2)&1)
					+ ((x2[n][j]>>1)&1)
					+ (x2[n][j]&1)
					+ ((x2[n][j-1]>>(s+1))&1))%2)<<s;
			} else {
				x2[n][j] += (((((x2[n][j])>>3)&1)
					+ ((x2[n][j]>>2)&1)
					+ ((x2[n][j]>>1)&1)
					+ (x2[n][j]&1))%2)<<s;
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

	memset(c_init,0,NOF_SUBFRAMES*sizeof(int));

	if (params.channel == PDSCH) { /* also PUSCH */
		for (n=0; n<NOF_SUBFRAMES; n++) {
			c_init[n] = (params.nrnti<<14)
				+ (params.q<<13)
				+ (n<<9)
				+ N_cell;
		}
	} else if (params.channel == PCFICH) {
		for (n=0; n<NOF_SUBFRAMES; n++) {
			c_init[n] = (((n+1) * (2*N_cell+1))<<9) + N_cell;
		}
	} else if (params.channel == PDCCH) {
		for (n=0; n<NOF_SUBFRAMES; n++) {
			c_init[n] = (n<<9) + N_cell;
		}
	} else if (params.channel == PBCH) {
		/* Caution: The scrambling sequence generator for the PBCH is
		 * not initialized on subframe basis */
		//for (n=0; n<NOF_SUBFRAMES; n++) {
			c_init[0] = N_cell;
		//}
	} else if (params.channel == PMCH) {
		for (n=0; n<NOF_SUBFRAMES; n++) {
			c_init[n] = (n<<9) + params.nMBSFN;
		}
	} else if (params.channel == PUCCH) {
		for (n=0; n<NOF_SUBFRAMES; n++) {
			c_init[n] = ((n+1) * (2*N_cell+1)<<16) + params.nrnti;
		}
	}
}

/**
 * @ingroup LTE Scrambling Sequence Generator
 * Generates the scrambling sequence based on the 3GPP specifications.
 * For the maximum input size (max number of input samples) it generates
 * all bits of the scrambling sequence for all 10 subframes.
 *
 * \param c Scrambling sequences for all 10 subframes and maximum input sample length
 * \param params Parameters that initialize the scrambling sequeznce generator
 */
inline void sequence_generation(unsigned (*c)[MAX_c], struct scrambling_params params)
{
	int i, n;
	unsigned c_init[NOF_SUBFRAMES];

	/* compute m-sequence {x1} */
	compute_x1();

	/* compute m-sequence {x2} */
	x2init(c_init, params);
	compute_x2(c_init);

	/* compute scrambling sequence {c} */
	for (n=0; n<NOF_SUBFRAMES; n++) {
		for (i=0; i<MAX_c; i++) {
			c[n][i] = x1[i+(params.Nc/32)] ^ x2[n][i+(params.Nc/32)];
		}
	}
}

/**
 * @ingroup Char-Integer Scrambling Function Call
 *
 * \param input Pointer to input data, input bit-sequence (char)
 * \param output Pointer to output data, scrambled bit-sequence (char)
 * \param N Number of input samples
 * \param c Pointer to scrambling sequence for a given subframe
 * \param direct Direct scrambling indicator
 */
inline void scramble(char *input, char *output, int N, unsigned *c, int direct, int sample)
{
	div_t js;

//	if (pbch) {
//		/* processing each radio frame */
//		if (sample == 0) {	/* new packet */
//			js.quot = 0;	/* quotient of integer division */
//			js.rem = 0;	/* remainder of integer division */
//		} else {
//			js = div(*sample,BIT_PER_INT);
//			direct = 1; /* necessary because 1st sample index may not be
//			multiple of 32 (bits per integer)*/
//		}
//	} else {
//		/* all other channels process samples each subframe and
//		 * scrambling sequence is reset each subframe */
//		js.quot = 0;
//		js.rem = 0;
//	}
	js = div(sample,BIT_PER_INT);
	if (direct) {
		direct_scrambling(input, output, N, c, js.quot, js.rem);
	} else {
		int_scrambling(input, output, N, c);
	}
}

/**
 * @ingroup Char-Integer Scrambling
 * Directly scrambles the input bit-sequence (char) with the scrambling
 * sequence (32-bit integers).
 *
 * \param input Pointer to input data, input bit-sequence (char)
 * \param output Pointer to output data, scrambled bit-sequence (char)
 * \param N Number of input samples
 * \param c Pointer to scrambling sequence for a given subframe
 */
inline void direct_scrambling(char *input, char *output, int N, unsigned *c, int j, int s)
{
	int i;
	int J = j;

	for (i=0; i<N; i++) {
		if (i == (j-J+1)*32) {
			s = 0;
			j++;
		}
		if (input[i] == ((c[j]>>s)&1)) {
	                output[i] = 0x0;
		} else {
			output[i] = 0x1;
		}
		s++;
	}
}

/**
 * @ingroup Integer Scrambling
 * Scrambles the input bit-sequence, transformed to integers with the scrambling
 * sequence (32-bit integers).
 * Assumes that sequence starts at sample 0
 *
 * \param input Pointer to input data, input bit-sequence (char)
 * \param output Pointer to output data, scrambled bit-sequence (char)
 * \param N Number of input samples
 * \param c Pointer to scrambling sequence for a given subframe
 */
inline void int_scrambling(char *input, char *output, int N, unsigned *c)
{
	int i;
	div_t ints;

	ints = div(N,BIT_PER_INT);
	ints.quot += 1;

	/* conversion of input sequence from chars to integers */
	char2int(input, input_ints, N);

	/* scramble with c */
	for (i=0; i<ints.quot; i++) {
		out_ints[i] = input_ints[i] ^ c[i];
	}

	/* conversion of scrambled sequence from integers to chars */
	int2char(out_ints, output, N, ints.rem);
}

/**
 * @ingroup Soft-bit Scrambling
 * Scrambles the input softbit-sequence (floats) with the scrambling
 * sequence (32-bit integers).
 *
 * \param input Pointer to input data, input sequence (float)
 * \param output Pointer to output data, scrambled sequence (float)
 * \param N Number of input samples
 * \param c Pointer to scrambling sequence for a given subframe
 * \param pbch PBCH indicator
 * \param frame Radio frame number (used for PBCH)
 * \param sample Initial sample (used for PBCH)
 */
inline void soft_scrambling(float *input, float *output, int N, unsigned *c, int sample)
{
	int i, j, s;
	div_t js;

//	if (pbch) {
//		/* processing each radio frame */
//		if (sample == 0) {	/* new packet */
//			js.quot = 0;	/* quotient of integer division */
//			js.rem = 0;	/* remainder of integer division */
//		} else {
//			js = div(*sample,BIT_PER_INT);
//		}
//	} else {
//		js.quot = 0;
//		js.rem = 0;
//	}
//	j = js.quot;
//	s = js.rem;

	js = div(sample,BIT_PER_INT);
	j = js.quot;
	s = js.rem;
	for (i=0; i<N; i++) {
		if (i == (j-js.quot+1)*32) {
			s = 0;
			j++;
		}
		if ((((input[i] > 0) && ((c[j]>>s)&1))
		|| ((input[i] <= 0) && ((c[j]>>s)&1)))) {
			output[i] = -input[i];
		} else {
			output[i] = input[i];
		}
		s++;
	}
}

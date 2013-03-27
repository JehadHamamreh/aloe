/* 
 * Copyright (c) 2012, Vuk Marojevic <marojevic@tsc.upc.edu>.
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
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>
#include <math.h>

#define INCLUDE_DEFS_ONLY
#include "lte_ctrl_ratematching.h"
#include "ctrl_ratematching.h"


char v[3][(int)INPUT_MAX_SAMPLES/3+1];
char w[INPUT_MAX_SAMPLES+31];
char e[OUTPUT_MAX_SAMPLES];
char m[(int)INPUT_MAX_SAMPLES/32+1][COLS];

int char_unwrap(char *input, char *out0, char *out1, char *out2, int int_l);
int subblock_interleaver(char *in_out, int in_l);
void bit_selection(char *input, char *output, int in_l, int out_l);


/**
 * @ingroup Unwraps input
 * Rate Matching at the LTE Transmitter (Tx)
 * Splits a single input stream into several output streams
 * That is, the input stream, recevied from the convolutional coder
 * and containing {d_i⁽⁰⁾, d_i⁽¹⁾, d_i⁽²⁾}, is split into 3 streams
 * d⁽⁰⁾, d⁽¹⁾, and d⁽²⁾
 *
 * \param input Pointer to input sequence (1D array of chars)
 * \param out0 Pointer to first output bit-sequence (1D array of chars)
 * \param out1 Pointer to second output bit-sequence (1D array of chars)
 * \param out2 Pointer to third output bit-sequence (1D array of chars)
 * \param insize Number of input samples (# chars)
 */
inline int char_unwrap(char *input, char *out0, char *out1, char *out2, int in_l)
{
	int i;

	in_l /= 3;
	for (i=0; i<in_l; i++) {
		*(out0++)=*(input++);
		*(out1++)=*(input++);
		*(out2++)=*(input++);
	}
	return in_l;
}

/**
 * @ingroup Subblock interleaver
 * Interleaves the input subblock
 *
 * \param input Pointer to input sequence (1D array of chars)
 * \param out0 Pointer to first output bit-sequence (1D array of chars)
 * \param out1 Pointer to second output bit-sequence (1D array of chars)
 * \param out2 Pointer to third output bit-sequence (1D array of chars)
 * \param insize Number of input samples (# chars)
 */
inline int subblock_interleaver(char *in_out, int in_l)
{
	int i, j;
	int dummies;
	int rows = intceil(in_l, COLS);
	int Kp = rows*COLS;

	/* Fill interleaver matrix m, row-wise */
	if (Kp > in_l) {
		dummies = Kp-in_l;
		/* 1st row */
		for (i=0; i<dummies; i++) {
			m[0][i] = DUMMYBIT;
		}
		for (i=dummies; i<COLS; i++) {
			m[0][i] = in_out[i-dummies];
		}
		/* Remaining rows*/
		for (i=1; i<rows; i++) {
			memcpy(m[i], &in_out[i*COLS-dummies], COLS);
		}
	} else {
		for (i=0; i<rows; i++) {
			memcpy(m[i], in_out, COLS);
		}
	}

	/* Permute columns of interleaver matrix m and read out column-wise */
	for (j=0; j<COLS; j++) {
		for (i=0; i<rows; i++) {
			in_out[j*rows+i] = m[i][PERM[j]];
		}
	}

	return Kp;
}

/**
 * @ingroup Bit selection and pruning
 * Selects the bits and prunes or extends the bit-sequence as necessary to
 * fill out_l output samples
 *
 * \param input Pointer to input sequence (1D array of type input_t)
 * \param output Pointer to output sequence (1D array of type output_t)
 * \param in_l Number of input samples
 * \param out_l Number of output samples
 */
inline void bit_selection(char *input, char *output, int in_l, int out_l)
{
	int i, j;
	int k = 0;

	while (k < out_l) {
		for (i=0; i<in_l; i++) {
			if (input[i] != DUMMYBIT) {
				output[k++] = input[i];
				if (k == out_l) {
					return;
				}
			}
		}
	}
}

/**
 * @ingroup Control information rate matching at the transmitter
 * Unwraps the input stream into 3 streams, interleaves the three subblocks,
 * joins and selects/prunes bits
 *
 * \param input Pointer to input sequence (1D array of input_t)
 * \param output Pointer to output sequence (1D array of output_t)
 * \param in_l Number of input samples
 * \param out_l Number of output samples
 */
inline void rate_matching(char *input, char *output, int in_l, int E)
{
	int i;
	int interleaver_in_l, interleaver_out_l;

	/* Unwraps input stream in three streams v[1], v[2] and v[3]*/
	interleaver_in_l = char_unwrap(input, v[0], v[1], v[2], in_l);

	/* Subblock Interleaving */
	for (i=0; i<3; i++) {
		interleaver_out_l = subblock_interleaver(v[i], interleaver_in_l);
	}

	/* Bit collection */
	for (i=0; i<3; i++) {
		memcpy(&w[i*interleaver_out_l], v[i], interleaver_out_l);
	}

	/* Bit selection and pruning */
	bit_selection(w, output, 3*interleaver_out_l, E);

}

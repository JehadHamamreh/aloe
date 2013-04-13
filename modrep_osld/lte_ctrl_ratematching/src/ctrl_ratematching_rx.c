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

float v_f[3][(int)OUTPUT_MAX_SAMPLES/3+31];
float vv[(int)OUTPUT_MAX_SAMPLES/3+31];
float w_f[OUTPUT_MAX_SAMPLES];
float m_f[(int)OUTPUT_MAX_SAMPLES/COLS+1][COLS];

/* Dummy bit insertion pattern for corresponding deinterleaver */
const int Pd[32] = {16, 0, 24, 8, 20, 4, 28, 12, 18,
		2, 26, 10, 22, 6, 30, 14, 17,
		1, 25, 9, 21, 5, 29, 13, 19,
		3, 27, 11, 23, 7, 31, 15};


int float_wrap(float *in0, float *in1, float *in2, float *out, int insize);
void subblock_deinterleaver(float *in, int in_l);
//void bit_collection(input_t *input, output_t *output, int in_l, int out_l);

//int float_wrap (float * in0, float * in1, float * in2, float * out, int insize);
//void subblock_deinterleaver(float *in, float *out, int in_l, int *out_l);

/**
 * @ingroup Wraps input
 * Used by the rate matching module at the LTE Recevier (Rx)
 * Joins input streams into a single output stream
 * That is, the 3 input streams in0, in1, and in2 are wrapped to form a single
 * output stream to be passed to the convolutional decoder
 *
 * \param input Pointer to input sequence (1D array of floats)
 * \param out0 Pointer to first output bit-sequence (1D array of floats)
 * \param out1 Pointer to second output bit-sequence (1D array of floats)
 * \param out2 Pointer to third output bit-sequence (1D array of floats)
 * \param insize Number of input samples (# floats)
 */
inline int float_wrap(float *in0, float *in1, float *in2, float *out, int insize)
{
	int i;

	for (i=0; i<insize; i++) {
		*(out++)=*(in0++);
		*(out++)=*(in1++);
		*(out++)=*(in2++);
	}
	return insize*3;
}

/**
 * @ingroup Subblock deinterleaver
 * Deinterleaves the input stream
 * Part of the control rata matching in reception mode
 * Adds dummy bits if necessary to fill the deinterleaver matrix
 * and removes them before writing out the deinterleaved stream
 *
 * \param in_out Pointer to input and output sequence (1D array of floats)
 * \param in_l Number of input samples (# floats)
 */
inline void subblock_deinterleaver(float *in_out, int in_l)
{
	int i, j;
	int idx;
	int dummies;
	int rows = intceil(in_l, COLS);
	int Kp = rows*COLS;

	/* Add dummy bits if necessary */
	dummies = Kp-in_l;
	if (dummies > 0) {
		memset(vv, 0, Kp*sizeof(float));
		for (i=0; i<dummies; i++) {
			vv[Pd[i]*rows] = DUMMYBIT;
		}
		idx = 0;
		for (j=0; j<Kp; j++) {
			if (vv[j] == 0) {
				vv[j] = in_out[j-idx];
			} else {
				idx++;
			}
		}
	} else {
		memcpy(vv, in_out, in_l*sizeof(float));
	}

	/* Fill interleaver matrix m, column-wise, inverting column permutation */
	for (j=0; j<COLS; j++) {
		for (i=0; i<rows; i++) {
			m_f[i][PERM[j]] = vv[j*rows+i];
		}
	}

	/* Read out row-wise */
	for (i=0; i<rows; i++) {
		memcpy(&vv[i*COLS], m_f[i], COLS*sizeof(float));
	}

	memcpy(in_out, &vv[dummies], in_l*sizeof(float));
}

/**
 * @ingroup Control information rate matching at the receiver
 * Prunes/extends input stream, deinterleaves the three subblocks
 * and writes out a single stream
 *
 * \param input Pointer to input sequence (1D array of input_t)
 * \param output Pointer to output sequence (1D array of output_t)
 * \param in_l Number of input samples
 * \param S Number of output samples
 */
inline int rate_unmatching(float *input, float *output, int in_l, int S)
{
	int i;
	int Kw = S/3;
	int out_l;

	memcpy(w_f, input, in_l*sizeof(float));
	if (S > in_l) {
		memset(&w_f[in_l], 0, (S-in_l)*sizeof(float));
	}

	/* sample stream split into 3 streams */
	for (i=0; i<3; i++) {
		memcpy(v_f[i], &w_f[i*Kw], Kw*sizeof(float));
	}

	/* Subblock Deinterleaving */
	for (i=0; i<3; i++) {
		subblock_deinterleaver(v_f[i], Kw);
	}

	/* Wraps input streams into single streams */
	out_l = float_wrap(v_f[0], v_f[1], v_f[2], output, Kw);

	return out_l;	/* same as S */
}

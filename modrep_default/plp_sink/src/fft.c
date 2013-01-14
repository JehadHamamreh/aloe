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

#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include <string.h>

#define INCLUDE_DEFS_ONLY
#include "plp_sink.h"

#define NOF_PLANS 5

static int fft_size[NOF_PLANS];
static int is_complex;
static double norm[NOF_PLANS];
static int nof_plans;


fftwf_complex *c_in_fft[NOF_PLANS], *c_out_fft[NOF_PLANS];
fftwf_plan c_plan[NOF_PLANS];

fftw_plan r_plan[NOF_PLANS];
double *r_in_fft[NOF_PLANS], *r_out_fft[NOF_PLANS];

int fft_init(int _nof_plans, int *_size, int _is_complex) {
	int i;

	nof_plans = _nof_plans;

	if (nof_plans > NOF_PLANS) {
		return -1;
	}

	is_complex = _is_complex;

	for (i=0;i<nof_plans;i++) {
		fft_size[i] = _size[i];

		if (is_complex) {
			c_in_fft[i] = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*fft_size[i]);
			c_out_fft[i] = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*fft_size[i]);
			c_plan[i] = fftwf_plan_dft_1d(fft_size[i], c_in_fft[i], c_out_fft[i], -1, 0);
			if (!c_plan[i]) {
				return -1;
			}
		} else {
			r_in_fft[i] = (double*) fftw_malloc(sizeof(double)*fft_size[i]);
			r_out_fft[i] = (double*) fftw_malloc(sizeof(double)*fft_size[i]);
			r_plan[i] = fftw_plan_r2r_1d(fft_size[i], r_in_fft[i], r_out_fft[i],
					FFTW_R2HC,0);
			if (!r_plan[i]) {
				return -1;
			}
		}

		/* Normalizatize DFT/IDFT output*/
		norm[i] = 1/sqrt(fft_size[i]);

	}

	return 0;
}

int c_fft_execute(void **inp, double *pl_signals, int *signal_lengths) {
	int i,j,p,len;

	for (i=0;i<NOF_INPUT_ITF;i++) {
		if (signal_lengths[i]) {
			for (p=0;p<nof_plans;p++) {
				if (signal_lengths[i] == fft_size[p])
					break;
			}
			if (p==nof_plans)
				p=0;

			len = fft_size[p];

			memset(c_in_fft[p],0,sizeof(_Complex float)*len);


			if (inp[i]) {
				memcpy(c_in_fft[p],inp[i],len*sizeof(_Complex float));

				fftwf_execute(c_plan[p]);

				for (j=0;j<len;j++) {
					c_out_fft[p][j] /= len;
					pl_signals[i*INPUT_MAX_SAMPLES+j] =
							10*log10((__real__ c_out_fft[p][j])*(__real__ c_out_fft[p][j])+
							(__imag__ c_out_fft[p][j])*(__imag__ c_out_fft[p][j]));
					if (isinf(-pl_signals[i*INPUT_MAX_SAMPLES+j])) {
						pl_signals[i*INPUT_MAX_SAMPLES+j] = -60;
					}
				}
			} else {
				memset(&pl_signals[i*INPUT_MAX_SAMPLES],0,sizeof(double)*len);
			}
		}
	}
	return len;
}


int r_fft_execute(void **inp, double *pl_signals, int *signal_lengths) {
	int p,i,j,len;
	float *input;

	for (i=0;i<NOF_INPUT_ITF;i++) {
		for (p=0;p<nof_plans;p++) {
			if (signal_lengths[i] == fft_size[p])
				break;
		}
		if (p==nof_plans)
			p=0;
		len=fft_size[p];

		if (inp[i]) {
			input = inp[i];
			for (j=0;j<len;j++) {
				r_in_fft[p][j] = (double) input[j];
			}

			fftw_execute(r_plan[p]);

			pl_signals[0] = r_out_fft[p][0]*r_out_fft[p][0];
			for (j=0;j<(len+1)/2-1;j++) {
				r_out_fft[p][j] /= len;
				r_out_fft[p][len-j-1] /= len;
				pl_signals[i*INPUT_MAX_SAMPLES+j] =
						r_out_fft[p][j]*r_out_fft[p][j]+
						r_out_fft[p][len-j-1]*r_out_fft[p][len-j-1];
				pl_signals[i*INPUT_MAX_SAMPLES+j] = 10*log10(pl_signals[i*INPUT_MAX_SAMPLES+j]);
				if (isinf(-pl_signals[i*INPUT_MAX_SAMPLES+j])) {
					pl_signals[i*INPUT_MAX_SAMPLES+j] = -30;
				}
			}
			if (len % 2 == 0) {
				pl_signals[i*INPUT_MAX_SAMPLES+len/2-1] = 20*log10(r_out_fft[p][len/2]);
				if (isinf(-pl_signals[i*INPUT_MAX_SAMPLES+len/2-1])) {
					pl_signals[i*INPUT_MAX_SAMPLES+len/2-1] = -30;
				}
			}
		} else {
			memset(&pl_signals[i*INPUT_MAX_SAMPLES],0,sizeof(double)*len);
		}
	}
	return len;
}

int fft_execute(void **inp, double *pl_signals, int *signal_lengths) {
	int i,j,len;
	float *input;

	if (is_complex) {
		return c_fft_execute(inp,pl_signals,signal_lengths);
	} else {
		return r_fft_execute(inp,pl_signals,signal_lengths);
	}
}

void fft_destroy() {
	int p;

	for (p=0;p<nof_plans;p++) {
		if (is_complex) {
			fftwf_destroy_plan(c_plan[p]);
			fftwf_free(c_in_fft[p]);
			fftwf_free(c_out_fft[p]);
		} else {
			fftw_free(r_in_fft[p]);
			fftw_free(r_out_fft[p]);
			fftw_destroy_plan(r_plan[p]);
		}
	}
}




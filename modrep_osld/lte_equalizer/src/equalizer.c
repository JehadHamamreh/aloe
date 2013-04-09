/*
 * Copyright (c) 2012,
 * Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * Xavier Arteaga
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
#include <string.h>
#include <complex.h>
#include <stdio.h>

#include "lte_lib/grid/base.h"
#include "equalizer.h"

/* Uncomment next line to enable debug mode */
//#define DEBUG

/* Useful macros */
#define intfloor(X, Y) ((X%Y>0)?((X-X%Y)/Y):(X/Y))
#define intceil(X, Y)  ((X%Y>0)?((X-X%Y)/Y+1):(X/Y))

int filter2d_init (filter2d_t* filter)
{

	int i, j;
	int ntime = filter->ntime;
	int nfreq = filter->nfreq;

	/* Filter input parameters */
	if (ntime > MAXOFDMF)
		return (-1);
	else if (nfreq > MAXSUBCF)
		return (-2);

	/* Load the 2-D interpolation mesh */
	for (i=0; i<ntime; i++){
		for (j=0; j<nfreq; j++){
			if (j < nfreq/2)
				filter->mesh[i][j]=(j+1.0)/(4.0*intceil(nfreq, 2));

			else if (j == intfloor(nfreq, 2))
				filter->mesh[i][j] = 0.25;

			else if (j > nfreq/2)
				filter->mesh[i][j]=(nfreq-j)/(4.0*intceil(nfreq, 2));
		}
	}

	for (i=0;i<filter->ntime;i++) {
		memset(filter->channel[i],0,sizeof(complex_t)*MAX_FFT_SIZE);
	}

#ifdef DEBUG
	/* Print 2-D filter */
	printf("2-D Filter: \n");
	for (i=0; i<ntime; i++){
		for (j=0; j<nfreq; j++){
			printf("%.3f ", filter[nfreq*i+j]);
		}
		printf("\n");
	}
	printf("\n")
#endif
	
	return 0;
}

void filter2d_reset(int symbol_sz, int nof_symbols, filter2d_t *filter) {
	int l;

	for (l=0;l<filter->ntime;l++) {
		memcpy(filter->channel[l],
				filter->channel[nof_symbols],sizeof(complex_t)*symbol_sz);
	}
	for (;l<filter->ntime+nof_symbols;l++) {
		memset(&filter->channel[l], 0, sizeof(complex_t)*symbol_sz);
	}
}

void filter2d_work(int k, int l, complex_t h, filter2d_t *filter) {
	int i,j;

	int ntime = filter->ntime;	// 2-D Filter size in time domain (number of ofdm symbols)
	int nfreq = filter->nfreq;	// 2-D Filter size in frequency domain (number of subcarriers)

	for (i=0; i<ntime; i++)	// Time loop
	{
		for (j=0; j<nfreq; j++)	// Frequency Loop
		{
			filter->channel[i+l][j+k-nfreq/2] +=
					h*(complex_t)(filter->mesh[i][j]);
		}
	}
}


/**
 * FIXME: It is assumed that equalizer->nfreq < frequency guard in the channel buffer
 */
void equalizer (refsignal_t *refsignal,
				int subframe_idx,
				complex_t*		input,		// Input frame (without equalization)
				complex_t*		output,		// Output frame (equalized)
				filter2d_t* filter,
				struct lte_grid_config *config
		)
{
	int k, mp, m, l, lp, ns;

	struct lte_symbol lte_symbol;

	complex_t h;	// It is for the channel correction
	complex_t known_ref, channel_ref;

	filter2d_reset(config->fft_size,config->nof_osymb_x_subf,filter);

	printf("a=[");
	/* Estimate the channel */
	for (l=0;l<config->nof_osymb_x_subf;l++) {

		if (lte_symbol_has_refsig(refsignal->port_id, l, config)) {
			lte_symbol.subframe_id = subframe_idx;
			lte_symbol.symbol_id = l;

			ns = lte_get_ns(&lte_symbol,config);
			lp = lte_refsig_l(l,config);
			for (m=0;m<config->nof_rs_x_symb;m++) {
				mp=lte_refsig_mp(m,config);
				k = refsignal->k[lp][ns][mp];

				known_ref = refsignal->signal[lp][ns][mp];
				channel_ref = input[l*config->fft_size+k];

				if (channel_ref != 0) {
					h = known_ref/channel_ref;
				} else {
					__real__ h = 0.0;
					__imag__ h = 0.0;
				}
				printf("%g+%gi, ",__real__ h, __imag__ h);

				filter2d_work(k,l,h,filter);
			}
		}
	}
	printf("];\n");

	/* Equalize the channel */
	printf("c=[");
	for (l=0;l<config->nof_osymb_x_subf;l++) {
		for (k=0; k<config->fft_size; k++){
			output[l*config->fft_size+k] =
					input[l*config->fft_size+k]*filter->channel[l][k];
			printf("%g+%gi, ",__real__ filter->channel[l][k], __imag__ filter->channel[l][k]);
		}
	}
	printf("];\n");

}

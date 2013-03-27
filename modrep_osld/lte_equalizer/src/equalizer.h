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


#ifndef ESTIMATOR_H_
#define ESTIMATOR_H_

#define DEFAULT_NTIME	7	// 2-D filter time dimension size
#define DEFAULT_NFREQ	11	// 2-D filter freq dimension size

/* Linear 2-D Filter declaration */
#define MAXOFDMF 35		// 2-D Filter maximum time domain size (in ofdm symbols)
#define MAXSUBCF 27		// 2-D Filter maximum frequency domain size (in subcarriers)

typedef _Complex float complex_t;

typedef struct {
	int ntime;	// 2-D Filter size in time domain (number of ofdm symbols)
	int nfreq;	// 2-D Filter size in frequency domain (number of subcarriers)
	float mesh [MAXOFDMF][MAXSUBCF];
	complex_t channel [NOF_OSYMB_X_SLOT_NORMAL+MAXOFDMF][MAX_FFT_SIZE];
} filter2d_t;

int filter2d_init (filter2d_t* equalizer);

void equalizer (	refsignal_t *refsignal,
					int subframe_idx,
					complex_t*		input,		// Input frame (without equalization)
					complex_t*		output,		// Output frame (equalized)
					filter2d_t* equalizer,
					struct lte_grid_config *config
	);
#endif // ESTIMATOR_H_

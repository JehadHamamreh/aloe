/* 
 * Copyright (c) 2012, Xavier Arteaga, Antoni Gelonch <antoni@tsc.upc.edu> &
 * Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
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
#include <complex.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "lte_resource_mapper.h"
#include "lte_lib/grid.h"
#include "refSignals.h"
#include "syncSignals.h"
#include "allocate_signals.h"

extern int nof_output_itf;

#define MIN_CELLID 						0
#define MAX_CELLID						503
#define MAX_FFT_SIZE 						2048
#define MAX_NOF_FFT						140
#define MAX_NOF_FFT_X_LTESLOT			14
#define NOF_SLOTS_X_FRAME				20
#define NOF_OFDM_SYMB_X_SLOT_SHORT_CP 	7
#define NOF_OFDM_SYMB_X_SLOT_LONG_CP	6
#define NOF_LTESLOTS_X_FRAME					20

int fft_size;
int cell_id;
int direction;
static int tslot_idx;
int lteslots_x_timeslot;
int nof_fft_x_lteslot;
int nof_subcarriers;
int nof_symbols_x_fft[MAX_NOF_FFT];
int nof_symbols_x_lte_slot[NOF_SLOTS_X_FRAME];

pmid_t tslot_idx_id;

/** OFDM_grid: Defines contents of each carrier the grid frame*/
char OFDM_grid[MAX_NOF_FFT*MAX_FFT_SIZE];

/** OFDM_grid: Output LTE frame */
output_t gridSLOT[MAX_NOF_FFT*MAX_FFT_SIZE];

extern _Complex float PSSsymb[PSSLEN];
extern float SSSseq[SSSLEN];

/**
 * @ingroup lte_resource_mapper
 *
 * \param direction 0 for the transmitter side, 1 for the receiver side
 * \param lteslots_x_timeslot Number of lte slots per execution time slot
 * \param (optional) fft_size, default is 128
 * \param (optional) cell_id, default is 0
 * \param (optional) cp_long, default is 0, set to 1 for long cyclic prefix
 * \param (optional) tslot_idx, if not provided, tslot_idx is counted automatically
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int i, j;
	int size, err, tslot_idx;
	int nof_crs_x_slot;
	int nof_ofdm_symb_x_frame;
	int cp_long;

	if (param_get_int_name("direction",&direction)) {
		moderror("Parameter direction must be specified\n");
		return -1;
	}

	if (!direction) {
		nof_output_itf = 1;
	} else {
		nof_output_itf = 2;
	}

	if (param_get_int_name("lteslots_x_timeslot",&lteslots_x_timeslot)) {
		moderror("Parameter lteslots_x_timeslot must be specified\n");
		return -1;
	}

	tslot_idx_id = param_id("tslot_idx");
	tslot_idx = 0;

	if (param_get_int_name("fft_size", &fft_size)) {
		fft_size = 128;
	}

	switch(fft_size) {
	case 128: 	nof_subcarriers=72; 	break;
	case 256: 	nof_subcarriers=180; 	break;
	case 512:	nof_subcarriers=300;	break;
	case 1024:	nof_subcarriers=600;	break;
	case 1536:	nof_subcarriers=900;	break;
	case 2048:	nof_subcarriers=1200;	break;
	default:
		moderror_msg("Parameter fft_size=%d not supported\n",fft_size);
		return -1;
	}

	if (param_get_int_name("cell_id", &cell_id)) {
		cell_id = MIN_CELLID;
	}

	if(cell_id < MIN_CELLID || cell_id > MAX_CELLID ){
		moderror_msg("Invalid cell_id=%d\n", cell_id);
		return -1;
	}

	if (param_get_int_name("cp_long", &cp_long)) {
		cp_long = 0;
	}

	if (cp_long) {
		nof_fft_x_lteslot = NOF_OFDM_SYMB_X_SLOT_LONG_CP;
	} else {
		nof_fft_x_lteslot = NOF_OFDM_SYMB_X_SLOT_SHORT_CP;
	}

	memset(gridSLOT, 0, sizeof(_Complex float)*MAX_NOF_FFT*MAX_FFT_SIZE);

	nof_crs_x_slot=(nof_subcarriers/6)*2;
	nof_ofdm_symb_x_frame = NOF_SLOTS_X_FRAME*nof_fft_x_lteslot;

	/* Generates grid pattern */
	err = setGrid (nof_ofdm_symb_x_frame, nof_subcarriers, fft_size, cell_id, OFDM_grid);
	if (err<0) return err;

	/**Print GRID*/
	/*printGrid (nof_ofdm_symb_x_frame, fft_size, OFDM_grid);*/

	/** Calculates the number of modulated symbols required at each OFDM symbol in the grid*/
	err=nofMSymbolsReq(OFDM_grid, nof_symbols_x_fft, nof_ofdm_symb_x_frame, fft_size);
	if (err<0) return err;

	/** Calculates the number of modulated symbols required at each slot in the LTE frame*/
	err=nofMSymbolxSLOT(nof_symbols_x_fft, nof_symbols_x_lte_slot, nof_ofdm_symb_x_frame,
			NOF_LTESLOTS_X_FRAME);
	if (err<0) return err;

	/** Generates Reference Signals*/
	err=setCRefSignals(nof_ofdm_symb_x_frame, nof_subcarriers, cell_id, CRSsymb);
	if (err<0) return err;

	/** Generates Primary Synchronization Signals*/
	err=setPSS(cell_id, PSSsymb, -1);
	if (err<0) return err;

	/** Generates Secondary Synchronization Signals*/
	loadmtable (m0s, m1s);
	err=setSSS(cell_id, SSSseq, m0s, m1s);
	if (err<0) return err;

	/** Adds CRS, PSS & SSS Signals to LTE Frame grid*/
	for(tslot_idx=0; tslot_idx<NOF_LTESLOTS_X_FRAME; tslot_idx++)
	{
		/** Allocates Reference Signals*/
		allocateCRS(OFDM_grid, fft_size, tslot_idx,
			nof_fft_x_lteslot, CRSsymb, gridSLOT);

		/** Allocates Primary Synchro Signals (PSS)*/
		allocatePSS(OFDM_grid, fft_size,\
			tslot_idx, nof_fft_x_lteslot, PSSsymb, gridSLOT);

		/** Allocates Secondary Synchro Signals (SSS)*/
		allocateSSS(OFDM_grid, fft_size,\
			tslot_idx, nof_fft_x_lteslot, SSSseq, gridSLOT);
	}

	tslot_idx = 0;

	return 0;
}

/**
 * @ingroup lte_resource_mapper
 *
 *  Main DSP function
 *
 * This function allocates the data symbols of one slot (0.5 ms) in the corresponding place
 * of the LTE frame grid. During initialization phase CRS, PSS and SSS signals has been
 * incorporated and will no be modified during run phase.
 * PBCH (Physical Broadcast Channel), PCFICH (Physical Control Format Indicator Channel) &
 * PDCCH (Physical Downlink Control Channel) will be incorporated.
 *
 */
int work(void **inp, void **out) {
	int i,j,k;
	int rcv_samples, snd_samples;
	input_t *input;
	output_t *output;
	output_t *output_refsig;
	int nsamples_refsig;

	if (!get_input_samples(0)) {
		return 0;
	}

	if (tslot_idx_id) {
		param_get_int(tslot_idx_id, &tslot_idx);
	}

	moddebug("tslot_idx=%d\n",tslot_idx);

	input = inp[0];
	output = out[0];

	if (!direction) {
		rcv_samples = 0;
		for (i=tslot_idx*lteslots_x_timeslot;i<tslot_idx*lteslots_x_timeslot+lteslots_x_timeslot;i++) {
			rcv_samples += nof_symbols_x_lte_slot[i];
		}
	} else {
		rcv_samples = lteslots_x_timeslot*nof_fft_x_lteslot*fft_size;
	}

	if (rcv_samples != get_input_samples(0)) {
		moderror_msg("Received %d samples but expected %d (tslot_idx=%d, direction=%d)\n",
				get_input_samples(0),rcv_samples, tslot_idx,direction);
		return 0;
	}

	if (!direction) {
		j=0;
		for (i=tslot_idx*lteslots_x_timeslot;
				i<tslot_idx*lteslots_x_timeslot+lteslots_x_timeslot;i++) {
			allocateDataOneUser(OFDM_grid, fft_size, i,
							nof_fft_x_lteslot, &input[j], gridSLOT);
			j += nof_symbols_x_lte_slot[i];
		}

		snd_samples = lteslots_x_timeslot*nof_fft_x_lteslot*fft_size;
		memcpy(output,&gridSLOT[tslot_idx*snd_samples], sizeof(input_t)*snd_samples);

		set_output_samples(0,snd_samples);
	} else {
		output_refsig = out[1];
		snd_samples = 0;
		nsamples_refsig = 0;
		k=0;
		for (i=tslot_idx*lteslots_x_timeslot;
				i<tslot_idx*lteslots_x_timeslot+lteslots_x_timeslot;i++) {
			snd_samples += deallocateDataOneUser (OFDM_grid, fft_size, i,
							nof_fft_x_lteslot, &input[k*nof_fft_x_lteslot*fft_size],
							&output[snd_samples]);
			if (output_refsig) {
				nsamples_refsig += deallocateCRS (OFDM_grid, fft_size,\
						i, nof_fft_x_lteslot, &input[k*nof_fft_x_lteslot*fft_size],
						&output_refsig[nsamples_refsig]);
			}
			k++;
		}

		set_output_samples(0,snd_samples);
		set_output_samples(1,nsamples_refsig);
	}

	tslot_idx++;
	if (tslot_idx==NOF_LTESLOTS_X_FRAME/lteslots_x_timeslot) {
		tslot_idx=0;
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


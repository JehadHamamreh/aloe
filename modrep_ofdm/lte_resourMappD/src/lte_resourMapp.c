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

#include "lte_resourMapp.h"
#include "grid.h"
#include "refSignals.h"
#include "syncSignals.h"


/** Module parameters*/
/** WE assume that these parameters can not be modified during run phase: HO COMENTEM?*/
int FFTsize;		/** 128, 256, 512, 1024, 1536, 2048*/
int CPmode; 		/** Cyclic Prefix mode: 0=normal, 1=extended*/
int cellID;		/** Physical Layer Cell Identity: A value between 0 and 503*/
int nofTxAntennas;	/*Number of Tx antennas mode: 1, 2, 4. ONLY ONE ANTENNA VERSION*/

/** Global Variables*/
int nofOFDMsymb;	/** Num. of OFDM symbols: 140 CPnormal, 120 CPextended*/
int nofOFDMsymbxSLOT;	/** Num. of OFDM symbols per slot*/
int nofsubC;		/** Num. of subcarriers: 72, 180, 300, 600, 900, 1200*/
int nofSYMBxSlot[NUMSLOTSxFRAME];	/** Num. of symbols for each 1 ms slot*/
int nofSYMBxFFT[MAXNUMOFDM];		/** Num. of data symbols x OFDM symbol*/
/** OFDM_grid: Defines contents of each carrier the grid frame*/
char OFDM_grid[MAXNUMOFDM*MAXFFTSIZE];
/** OFDM_grid: Output LTE frame */
output_t gridSLOT[MAXNUMOFDM*MAXFFTSIZE];

/** Extern defined variables*/
extern _Complex float PSSsymb[PSSLEN];
extern float SSSseq[SSSLEN];

/**
 * @ingroup template
 *  Document here the module's initialization parameters
 *
 * The documentation should explain which are the possible parameters, what they do and if they are
 * mandatory or optional (indicating the default value in such case).
 *
 * \param gain Document paramater gain
 * \param block_length Document parameter block_length
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int i, j;
	int size, err, nofSlot;
	int nofSymb, nofCRSxSlot=(nofsubC/6)*2;
	//int /*phylayerID=cellID%3,*/ phylayerIDgroup=cellID/NUMSSS;

	/** CPmode*/
	if (param_get_int_name("CPmode", &CPmode)) {
		CPmode = CPNORMAL;
	}
	/* Print CPmode value */
	modinfo_msg("Parameter CPmode=%d\n",CPmode);
	/** Check nofOFDMsymb value*/
	if(CPmode != 0 && CPmode != 1 ){
		moderror_msg("Invalid CPmode value %d\n", CPmode);
		return -1;
	}
	if(CPmode == 1 ){
		moderror_msg("Extended CP (CPmode=%d) not valid in current version\n", CPmode);
		return -1;
	}

	/** FFTsize*/
	if (param_get_int_name("FFTsize", &FFTsize)) {
		FFTsize = 128;
	}
	/* Print FFTsize value */
	modinfo_msg("Parameter FFTsize=%d\n",FFTsize);
	/** Check nofOFDMsymb value*/
	if(FFTsize != 128 && FFTsize != 256 && FFTsize != 512\
			&& FFTsize != 1024 && FFTsize != 1536 && FFTsize != 2048){
		moderror_msg("Invalid FFTsize value %d\n", FFTsize);
		return -1;
	}
	/** CellID*/
	if (param_get_int_name("cellID", &cellID)) {
		cellID = 0;
	}
	/* Print cellID value */
	modinfo_msg("Parameter cellID=%d\n",cellID);
	/** Check cellID value*/
	if(cellID < 0 || cellID > 503 ){
		moderror_msg("Invalid cellID value %d\n", cellID);
		return -1;
	}
	/** nofTxAntennas*/
	if (param_get_int_name("nofTxAntennas", &nofTxAntennas)) {
		nofTxAntennas = 1;
	}
	/* Print nofTxAntennas value */
	modinfo_msg("Parameter nofTxAntennas=%d\n",nofTxAntennas);
	/** Check nofTxAntennas value*/
	if(nofTxAntennas != 1){	//Preliminary
	//if(nofTxAntennas > 4 || nofTxAntennas < 1){	//Final
		moderror_msg("Invalid nofTxAntennas value %d\n",\
			nofTxAntennas);
		return -1;
	}
	/** Initialize the working LTE frame grid array*/
	memset(gridSLOT, 0, sizeof(_Complex float)*MAXNOFSYMBOLSSLOT*MAXFFTSIZE);

	/** Define the number of subcarriers*/
	if(FFTsize == 128)nofsubC=72;
	if(FFTsize == 256)nofsubC=180;
	if(FFTsize == 512)nofsubC=300;
	if(FFTsize == 1024)nofsubC=600;
	if(FFTsize == 1536)nofsubC=900;
	if(FFTsize ==2048)nofsubC=1200;

	nofCRSxSlot=(nofsubC/6)*2;
	modinfo_msg("nofsubC=%d, nofCRSxSlot=%d\n", nofsubC, nofCRSxSlot);

	if(CPmode == CPNORMAL){
		nofOFDMsymb = 140;
		nofOFDMsymbxSLOT=7;
	}
	else {
		nofOFDMsymb = 120;
		nofOFDMsymbxSLOT=6;
	}
	/* Generates grid pattern, ntrafffic is the number of available resources */
	err = setGrid (nofOFDMsymb, nofsubC, FFTsize, cellID, OFDM_grid);
	if (err<0) return err;

	/**Print GRID*/
	//printGrid (nofOFDMsymb, FFTsize, OFDM_grid);

	/** Calculates the number of modulated symbols required at each OFDM symbol in the grid*/
	err=nofMSymbolsReq(OFDM_grid, nofSYMBxFFT, nofOFDMsymb, FFTsize);
	if (err<0) return err;

	/** Calculates the number of modulated symbols required at each slot in the LTE frame*/
	err=nofMSymbolxSLOT(nofSYMBxFFT, nofSYMBxSlot, nofOFDMsymb, NUMSLOTSxFRAME);
	if (err<0) return err;

	/** Generates Reference Signals*/
	err=setCRefSignals(nofOFDMsymb, nofsubC, cellID, CRSsymb);
	if (err<0) return err;

	/** Generates Primary Synchronization Signals*/
	err=setPSS(cellID, PSSsymb, -1);
	if (err<0) return err;

	/** Generates Secondary Synchronization Signals*/
	loadmtable (m0s, m1s);
	err=setSSS(cellID, SSSseq, m0s, m1s);
	if (err<0) return err;

	/** Adds CRS, PSS & SSS Signals to LTE Frame grid*/
	for(nofSlot=0; nofSlot<NUMSLOTSxFRAME; nofSlot++)
	{
		/** Allocates Reference Signals*/
		nofSymb=allocateCRS(OFDM_grid, FFTsize,nofSlot,\
			nofOFDMsymbxSLOT, CRSsymb, gridSLOT);
		/** Allocates Primary Synchro Signals (PSS)*/
		nofSymb=allocatePSS(OFDM_grid, FFTsize,\
			nofSlot, nofOFDMsymbxSLOT, PSSsymb, gridSLOT);
		/** Allocates Secondary Synchro Signals (SSS)*/
		nofSymb=allocateSSS(OFDM_grid, FFTsize,\
			nofSlot, nofOFDMsymbxSLOT, SSSseq, gridSLOT);
	}

	return 0;
}

/**
 * @ingroup template
 *
 *  Main DSP function
 *
 * This function allocates the data symbols of one slot (0.5 ms) in the corresponding place
 * of the LTE frame grid. During initialization phase CRS, PSS and SSS signals has been
 * incorporated and will no be modified during run phase.
 * PBCH (Physical Broadcast Channel), PCFICH (Physical Control Format Indicator Channel) &
 * PDCCH (Physical Downlink Control Channel) will be incorporated.
 *
 * \param inp Input interface buffers. The value inp[i] points to the buffer received
 * from the i-th interface. The function get_input_samples(i) returns the number of received
 * samples (the sample size is by default sizeof(input_t))
 *
 * \param out Output interface buffers. The value out[i] points to the buffer where the samples
 * to be sent through the i-th interfaces must be stored.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 */
int work(void **inp, void **out) {
	int rcv_samples, snd_samples, err;
	int i, j;
	int nofSymb, numOFDM, nofCRSxSlot=(nofsubC/6)*2;
	static int nofSlot=0;
	input_t *input;
	output_t *output;

	if (param_get_int_name("slot_idx", &nofSlot)) {
		nofSlot=nofSlot%NUMSLOTSxFRAME; /** Identifies the number of slot in the frame*/
	}

	input = inp[0];
	output = out[0];
	rcv_samples = get_input_samples(0); /* get_input_samples(0) returns the samples received from input 0*/

	/** Policy: ALL AVAILABLE RESOURCES ALLOCATED TO ONE USER*/
	/** Check if enough data received. If not error. Other option: partial RB allocation?*/
	if(rcv_samples != nofSYMBxSlot[nofSlot]){
		moderror_msg("rcv_samples=%d != nofSYMBxSlot[nofSlot=%d]=%d\n",\
				rcv_samples, nofSlot, nofSYMBxSlot[nofSlot]);
		return -1;
	}

	/** Allocate data symbols*/
	nofSymb=allocateDataOneUser(OFDM_grid, FFTsize,\
			nofSlot, nofOFDMsymbxSLOT, input, gridSLOT);
	if(nofSymb != nofSYMBxSlot[nofSlot]){
			moderror_msg("nofSymb=%d != nofSYMBxSlot[nofSlot=%d]=%d\n",\
					nofSymb, nofSlot, nofSYMBxSlot[nofSlot]);
			return -1;
		}

	/**Rotate Spectrum & Send data*/
	numOFDM=nofSlot*nofOFDMsymbxSLOT;
	snd_samples=0;
	for (i=numOFDM; i<numOFDM+nofOFDMsymbxSLOT; i++){
		for(j=FFTsize/2; j<FFTsize; j++){
			*(output+snd_samples)=gridSLOT[i*FFTsize+j];
			snd_samples++;
		}
		for(j=0; j<FFTsize/2; j++){
			*(output+snd_samples)=gridSLOT[i*FFTsize+j];
			snd_samples++;
		}
	}

	nofSlot++;
	return snd_samples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


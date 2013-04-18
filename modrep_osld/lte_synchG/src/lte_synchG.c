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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

//#include "dft.h"
#include "lte_synchG.h"
#include "utils.h"
#include "dspfunct.h"
#include "PSSfilters.h"
#include "syncProcess.h"
#include "syncSignals.h"

#define SETPARAM

/** Module defines*/
#define BUFFERCsz 100*2048
_Complex float checkout[BUFFERCsz];	//ELIMINAR

/** Module params*/
//inputs
int block_length;
int FFTsize;
int LTEframe_structtype;
//Outputs
int PHYlayerCellID;	//Physical-layer cell identity: 0-503
int CPtype;		//Cyclic Prefix type
int nofsubframe;	//Current subframe number
int nofslot; 		//Current number of slot



/** Global variables*/
/**Circular buffer*/
_Complex float bufferC[BUFFERCsz];
buffctrl buffCtrl;

/** Low Pass Filter*/
extern float LPPSS_128filter[128];
extern float LPPSS_256filter[128];
extern float LPPSS_512filter[128];
extern float LPPSS_1024filter[128];
extern float LPPSS_1536filter[128];
extern float LPPSS_2048filter[128];

/** PSS Rx*/
_Complex float PSSRx_ID[PSSLEN+2];
/** SSS Rx*/
extern int ss[N], cc[N], zz[N];
int m0s[NUMSSS];
int m1s[NUMSSS];

/**Correlation*/
_Complex float coeff0[MAXFFTSIZE];
_Complex float coeff1[MAXFFTSIZE];
_Complex float coeff2[MAXFFTSIZE];
_Complex float correl[INPUT_MAX_SAMPLES+2048];

/** FFTs */
fftwf_plan fft64, fft128, fft256, ft512, fft1024, fft1536, fft2048;
fftwf_plan ifft64, ifft128, ifft256, ift512, ifft1024, ifft1536, ifft2048;

//Frame-Subframe-slot info control
synchctrl_t subfrmCtrl;
int slot_size, subframe_size;

int sf_pm_idx;

/**@ingroup lte_synch
 * This module captures the PSS synchronism, identifies the PSS sequence and the CP size.
 * From that aligns the received samples flow with the LTE subframe limits.
 * Identifies the subframe and slot number.
 * The output is of multiple of subframe size an aligned with their limits in the LTE frame.
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 */
int initialize() {

	//INPUT PARAMS
	if (param_get_int_name("block_length", &block_length)) {
			block_length = 0;
	}
	moddebug("Parameter block_length is %d\n",block_length);
	/* Verify control parameters */
	if (block_length > input_max_samples || block_length < 0) {
			moderror_msg("Invalid block length=%d > input_max_samples=%d\n", block_length, input_max_samples);
			return -1;
	}
	if (param_get_int_name("FFTsize", &FFTsize)) {
				FFTsize = 128;
	}
	moddebug("Parameter FFTsize is %d\n",FFTsize);
	/* Verify control parameters */
	if (FFTsize != 128 &&  FFTsize != 256 && FFTsize != 512 &&
		FFTsize != 1024 && FFTsize != 1536 && FFTsize != 2048) {
		moderror_msg("Invalid FFTsize %d\n", FFTsize);
		return -1;
	}
	if (param_get_int_name("LTEframe_structtype", &LTEframe_structtype)) {
		LTEframe_structtype = 1;
	}
	moddebug("Parameter LTEframe_structtype %d\n",LTEframe_structtype);
		/* Verify control parameters */
	if (LTEframe_structtype != 1 &&  LTEframe_structtype != 2) {
		moderror_msg("Invalid LTEframe_structtype %d\n", LTEframe_structtype);
		return -1;
	}
	//OUTPUT PARAMS
	if (param_get_int_name("PHYlayerCellID", &PHYlayerCellID)) {
		PHYlayerCellID = 127;
	}
	moddebug("Parameter PHYlayerCellID=%d\n",PHYlayerCellID);
	if (param_get_int_name("CPtype", &CPtype)) {
		CPtype = CP_UNKNOWN;
	}
	moddebug("Parameter CPtype=%d\n",CPtype);
	if (param_get_int_name("nofsubframe", &nofsubframe)) {
		nofsubframe = 0;
	}
	moddebug("Parameter nofsubframe=%d\n",nofsubframe);
	if (param_get_int_name("nofslot", &nofslot)) {
		nofslot = 0;
	}
	moddebug("Parameter nofslot=%d\n",nofslot);


	//////////////INITTIALIZE
	/** Set PSS Bandwidth filter*/
	setPSS_LPrealfilter128(LPPSS_128filter);
	setPSS_LPrealfilter512(LPPSS_512filter);

	/** Generate PSS time sequence 0: coeff of correlator*/
	genPSStime_seq(0, FFTsize, coeff0, 1);
	/** Generate PSS time sequence 0: coeff of correlator*/
	genPSStime_seq(1, FFTsize, coeff1, 1);
	/** Generate PSS time sequence 0: coeff of correlator*/
	genPSStime_seq(2, FFTsize, coeff2, 1);

	//Initialize Tables SSS
	loadmtable (m0s, m1s);
	initSSStables(zz, ss, cc);

	//FrameAlignment
	slot_size = SIZEOF_SLOT_CPNOR(FFTsize);
	subframe_size = slot_size*2;
	//Initialize subframe info&ctrl
	subfrmCtrl.CPtype = CP_NOR;
	subfrmCtrl.p2_subframe = 0;
	subfrmCtrl.nofsubframe = 0;
	subfrmCtrl.p2_OFDMPSS = 0;
	subfrmCtrl.nofOFDMsymb = 0;
	subfrmCtrl.nofslot = 0;
	subfrmCtrl.synchstate = SYNCH_INIT;

	// Initialize Circular Buffer*/
	memset(&bufferC, 0, sizeof(_Complex float)*BUFFERCsz);
	buffCtrl.writeIndex = subframe_size;
	buffCtrl.readIndex = subframe_size;
	buffCtrl.buffsize = BUFFERCsz;
	buffCtrl.occuplevel=0;

#ifdef _COMPILE_ALOE
	sf_pm_idx = oesr_get_variable_idx(ctx, "ctrl","subframe_rx");
	if (sf_pm_idx < 0) {
		moderror("Error getting remote parameter subframe_rx\n");
	}
	moddebug("Remote subframe_rx parameter is a at %d\n",sf_pm_idx)
#endif


	return 0;
}
int first=0;

//set_output_samples(1,1024) envia 1024 samples pel interface 1 (encomptes del 0)

//Les has de ficar al punter out[1]
//We assume the reception  at less of 14*FFTsize symbols


int work(void **inp, void **out) {
	int bypass;
	int j, k=0, n, t, rcv_samples, snd_samples;
	input_t *input;

	output_t *output;
	int numsubframes=1;

	buffctrl localbuffCtrl;
	int corr_detct=0;				//Correlation detected

	_Complex float aux[BUFFERCsz];
	int bufferroom;


		bypass=0;
		param_get_int_name("bypass",&bypass);

		rcv_samples = get_input_samples(0);
		if (!rcv_samples) {
			return 0;
		}
		input = inp[0];
		output = out[0];
		if (bypass) {
			memcpy(output,input,rcv_samples*sizeof(input_t));
			if (!first) {
#ifdef _COMPILE_ALOE
				if (sf_pm_idx >= 0) {
					if (param_remote_set(out, 1, sf_pm_idx, &first,
							sizeof(int))) {
						moderror("Setting parameter\n");
						return -1;
					}
				}
#endif
				first=1;
			}
			set_output_samples(0,rcv_samples);
			return 0;
		}

		moddebug("/////////////////WORK START\n",0);
//graph = out[1];
		snd_samples=0;

		//Check if enough room in C bufffer available
		bufferroom=buffCtrl.buffsize-buffCtrl.occuplevel;
		if(rcv_samples > bufferroom){
			moddebug("Excess of data received. rcv_samples=%d > Bufferroom =%d & \n", rcv_samples, bufferroom);
			return -1;
		}
		//Write C buffer
		writeCbuff(&buffCtrl, bufferC, input, rcv_samples);
		//Number of available subframes
		numsubframes = (buffCtrl.occuplevel/subframe_size);
		for(n=0; n<numsubframes; n++){
			localbuffCtrl.writeIndex = buffCtrl.writeIndex;
			localbuffCtrl.occuplevel = buffCtrl.occuplevel;
			moddebug("localbuffCtrl.readIndex = %d, buffCtrl.readIndex = %d, diff = %d\n",
					localbuffCtrl.readIndex, buffCtrl.readIndex, localbuffCtrl.readIndex - buffCtrl.readIndex);
			localbuffCtrl.readIndex = buffCtrl.readIndex;
			//Read C buffer
			readCbuff(&localbuffCtrl, bufferC, aux, subframe_size);
			//Initiate synchro
			corr_detct=initframe_alignment(subframe_size, FFTsize, aux, correl, coeff0, coeff1, coeff2, &subfrmCtrl);
			if(corr_detct==1){
				//Check CP
				subfrmCtrl.CPtype=detect_CP(subfrmCtrl.p2_OFDMPSS, FFTsize, aux);
				if(subfrmCtrl.CPtype == CP_NOR){
					subfrmCtrl.nofOFDMsymb = 7;			//Current OFDM symbol in the sequence
					subfrmCtrl.p2_OFDMPSS -= CP_NOR_SlotN(FFTsize);	//Point to the OFDM symbol including CP
					subfrmCtrl.p2_subframe = subfrmCtrl.p2_OFDMPSS - (slot_size - FFTsize-CP_NOR_SlotN(FFTsize));
					//subfrmCtrl.nofsubframe = 0;
					}

	//////////////////////////////////////
				//Detect SSS
				//subfrmCtrl.p2_OFDMSSS = subfrmCtrl.p2_OFDMPSS-FFTsize;	//Point to the SSS
				//printf("sizeof(_Complex float)=%d\n", sizeof(_Complex float));

				//detect_SSS (FFTsize, SSSsubfr0time, phyLayerCellID%3, m0s, m1s, &RxphyLayerCellID, &nofsubframe);
				//(_Complex float *)(aux - subfrmCtrl.p2_OFDMSSS)
				//printf("aux=%u\n", aux);
				/*for(i=0; i<1024;i++){
					__real__ checkout[i] =  __real__ aux[i];
				}
				*/
				if (detect_SSS (FFTsize, aux, subfrmCtrl.PSSseq, m0s, m1s, (synchctrl2_t *) &subfrmCtrl) == 1) {
#ifdef SETPARAM
				#ifdef _COMPILE_ALOE

					moddebug("ts: %d. Detected SSS subframe %d\n", oesr_tstamp(ctx),
							subfrmCtrl.nofsubframe);
					if (sf_pm_idx >= 0) {
						if (param_remote_set(out, 1, sf_pm_idx, &subfrmCtrl.nofsubframe,
								sizeof(int))) {
							moderror("Setting parameter\n");
							return -1;
						}
					}
				#endif
#endif
				}
				//moddebug("subfrmCtrl.p2_OFDMPSS=%d\n", subfrmCtrl.p2_OFDMPSS);
				//moddebug("subfrmCtrl.p2_OFDMSSS=%d\n", subfrmCtrl.p2_OFDMSSS);
				moddebug("subfrmCtrl.nofsubframe=%d\n", subfrmCtrl.nofsubframe);
				moddebug("subfrmCtrl.nofOFDMsymb=%d\n", subfrmCtrl.nofOFDMsymb);
				moddebug("subfrmCtrl.nofslot=%d\n", subfrmCtrl.nofslot);
				moddebug("subfrmCtrl.PSSseq=%d\n", subfrmCtrl.PSSseq);
				moddebug("subfrmCtrl.phyLayerCellID=%d\n", subfrmCtrl.phyLayerCellID);
				PHYlayerCellID=subfrmCtrl.phyLayerCellID;
				CPtype=subfrmCtrl.CPtype;
				nofsubframe=subfrmCtrl.nofsubframe;
				nofslot=subfrmCtrl.nofslot;

	//////////////////////////////////////
				//Making read pointer correction
				//moddebug(" subfrmCtrl.p2_subframe=%d\n", subfrmCtrl.p2_subframe);
				buffCtrl.readIndex += subfrmCtrl.p2_subframe;
				corr_detct=0;
			}
		//Send to output
		readCbuff(&buffCtrl, bufferC, output + k*subframe_size, subframe_size);
		//Increase nofsubframe index
		subfrmCtrl.nofsubframe = (subfrmCtrl.nofsubframe+1)%10;
		k++;
		snd_samples +=subframe_size;
		}

		set_output_samples(0,snd_samples);


		return 0;
}

int stop() {

	return 0;
}

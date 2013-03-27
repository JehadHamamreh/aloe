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

/*
 * Calculate PSS samples and save it in two arrays
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <fftw3.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>
#define INCLUDE_DEFS_ONLY
#include "lte_synchG.h"
#include "PSSfilters.h"
#include "syncSignals.h"
#include "syncProcess.h"


/**Define*/
#define qp (int)floor((float)id1/30.0)
#define q  (int)floor((float)(id1+qp*(qp+1)/2)/30.0)
#define m  (int)(id1 + q*(q+1)/2)

/** Global variables*/
int ss[N], cc[N], zz[N];

extern float LPPSS_128filter[128];
extern float LPPSS_256filter[128];
extern float LPPSS_512filter[128];
extern float LPPSS_1024filter[128];
extern float LPPSS_1536filter[128];
extern float LPPSS_2048filter[128];

/** PRIMARY SYNCH SIGNALS*/
/**
 * @brief Function documentation: setPSS()
 * This function calculates the Zadoff-Chu sequence.
 * @params
 * @params int phylayerID:(0, 1, 2) Physical Layer Identity within the
 * Physical Layer cell-Identity Group.
 * @params  _Complex float *PSSsymb: Output array.
 * @params int TxRxMode: -1 (Tx Mode), 1 (Rx Mode) .
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int setPSS(int CellID, _Complex float *PSSsymb, int TxRxMode)
{
	int i;
	float arg, rootidx;
	int phylayerID;

	phylayerID=CellID%3;
	if(phylayerID == 0)rootidx = PSSCELLID0;
	if(phylayerID == 1)rootidx = PSSCELLID1;
	if(phylayerID == 2)rootidx = PSSCELLID2;

	//moddebug("phylayerID=%d, cellID=%d, rootidx=%f TzRxmode=%3.3f\n", phylayerID, CellID, rootidx, TxRxMode);

	for(i=0; i<PSSLEN/2; i++){
		arg=((float)TxRxMode)*PI*rootidx*((float)i*((float)i+1.0))/63.0;
		__real__ PSSsymb[i]=cos(arg);
		__imag__ PSSsymb[i]=sin(arg);
	}
	for(i=PSSLEN/2; i<PSSLEN; i++){
		arg=((float)TxRxMode)*PI*rootidx*(((float)i+2.0)*((float)i+1.0))/63.0;
		__real__ PSSsymb[i]=cos(arg);
		__imag__ PSSsymb[i]=sin(arg);
	}
	return 1;
}

/**@ingroup genPSStime_seq
 * This module generate the PSS time sequence for the different FFT size
 * \param cellID: Identifies the sequence number: 0, 1, 2
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * \param TxRxmode: defines if the sequence generate is for Tx or Rx side
 */
int genPSStime_seq(int cellID, int FFTsize, fftwf_complex *PSS_time, int TxRxmode){

	int s, j, i;
	_Complex float PSS_ID[PSSLEN+2];
	/**FFT*/
	fftwf_complex PSS_freq[2048];
	fftwf_plan plan64, plan128, plan256, plan512, plan1024, plan1536, plan2048;

	/**Select cellID: 0, 1, 2*/
	setPSS(cellID, PSS_ID, TxRxmode);
	//TX PSS: ROTATE
	memset(PSS_freq, 0, sizeof(_Complex float)*FFTsize);
	s=1;	//DC at position O
	for(i=PSSLEN/2; i<PSSLEN; i++){
			PSS_freq[s] = PSS_ID[i];
			s++;
	}
	s=(FFTsize-(PSSLEN/2));
	for(i=0; i<PSSLEN/2; i++){
			PSS_freq[s] = PSS_ID[i];
			s++;
	}
	if(FFTsize==64){
			plan64 = fftwf_plan_dft_1d(64, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan64);
		}
	if(FFTsize==128){
			plan128 = fftwf_plan_dft_1d(128, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan128);
		}
	if(FFTsize==512){
			plan512 = fftwf_plan_dft_1d(512, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan512);
	}
	if(FFTsize==1024){
			plan1024 = fftwf_plan_dft_1d(1024, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan1024);
	}
	if(FFTsize==1536){
			plan1536 = fftwf_plan_dft_1d(1536, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan1536);
	}
	if(FFTsize==2048){
			plan2048 = fftwf_plan_dft_1d(2048, PSS_freq, PSS_time, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan2048);
	}
	return 0;
}


/** SECONDARY SYNCH SIGNALS*/
/**
 * @brief Function documentation: loadmtable()
 * This function generates the table 6.11.2.1-1 described at
 * 3GPP TS 36.211 version 10.5.0 Release 10.
 * @params
 * @params int *m0s:
 * @params int *m1s:
 */

void loadmtable (int *m0s, int *m1s){
	int id1;
	for (id1=0; id1<NUMSSS; id1++) m0s[id1] = m%N;
	for (id1=0; id1<NUMSSS; id1++) m1s[id1] = (m0s[id1]+(int)floor((float)m/31.0)+1)%N;
	// Uncomment next line to print the 'm' table
	//for (id1=0; id1<NUMSSS; id1++) printf("cell id %d -> m0 = %d; m1 = %d\n", id1, m0s[id1], m1s[id1]);
}

int getIdFromTable (int * m0s, int * m1s, int m0, int m1){
	int i;
	for (i=0; i<NUMSSS; i++){
		if ((m0s[i]==m0)&&(m1s[i]==m1)) return i;
	}
	return -1;
}


/**
 * @brief Function documentation: initSSStables()
 * This function generates the scrambling sequences required for generation of
 * SSS sequence according with 3GPP TS 36.211 version 10.5.0 Release 10.
 * @params
 * @params int *z:
 * @params int *s:
 * @params int *c:
 */
void initSSStables(int *z, int *s, int *c){

	int i;
	int x[N]={0,0,0,0,1};

	for (i=0; i<26; i++) x[i+5] = (x[i+2]+x[i])%2;
	for (i=0; i<N; i++) s[i] = 1-2*x[i];

	for (i=0; i<26; i++) x[i+5] = (x[i+3]+x[i])%2;
	for (i=0; i<N; i++) c[i] = 1-2*x[i];

	for (i=0; i<26; i++) x[i+5] = (x[i+4]+x[i+2]+x[i+1]+x[i])%2;
	for (i=0; i<N; i++) z[i] = 1-2*x[i];
}

/**
 * @brief Function documentation: setSSS()
 * Algorithm taken from  3GPP TS 36.211 version 10.5.0 Release 10 Section 6.11.2
 * @params
 * @params int phylayerCellID:Physical Layer Cell-Identity (0-503)
 * @params float *SSSseq: SSS sequence.
 * @params int *m0s: mo index.
 * @params int *m1s: m1 index.
 *
 * @return On success returns 1.
 * On error returns -1.
 */
int setSSS(int phylayerCellID, float *SSSsubfr0, float *SSSsubfr5, int *m0s, int *m1s){
	int i, j, id1 = phylayerCellID/3, id2 = phylayerCellID%3;
	//int ss[N], x[N], cc[N], zz[N], ss0[N], ss1[N], cc0[N], cc1[N], zz0[N], zz1[N];
	int ss0[N], ss1[N], cc0[N], cc1[N], zz0[N], zz1[N];
	int m0 = m0s[id1];
	int m1 = m1s[id1];

	moddebug("CellID=%d, Cell-Group ID=%d, m0=%d, m1=%d\n", id1, id2, m0, m1);

	if(phylayerCellID >= MAXPHYLAYERCELLID)return -1;

	//initSSStables(zz, ss, cc);
	for(i=0; i<N; i++){
		ss0[i] = ss[(i+m0)%N];
		ss1[i] = ss[(i+m1)%N];
		zz0[i] = zz[(i+m0%8)%N];
		zz1[i] = zz[(i+m1%8)%N];
		cc0[i] = cc[(i+id2)%N];
		cc1[i] = cc[(i+id2+3)%N];
	}

	for (i=0; i<N; i++){
		// Even Resource Elements: Sub-frame 0
		SSSsubfr0[2*i] = (float)(ss0[i]*cc0[i]);
		// Odd Resource Elements: Sub-frame 0
		SSSsubfr0[2*i+1] = (float)(ss1[i]*cc1[i]*zz0[i]);
	}
	for (i=0; i<N; i++){
		// Even Resource Elements: Sub-frame 5
		SSSsubfr5[2*i]   = (float)(ss1[i]*cc0[i]);
		// Odd Resource Elements: Sub-frame 5
		SSSsubfr5[2*i+1] = (float)(ss0[i]*cc1[i]*zz1[i]);
	}
//	for(i=0; i<2*N; i++){
//		printf("SSSsubfr0_par[%d]=%02.2f, SSSsubfr0_senar[%d]=%02.2f, s0=%d, s1=%d, c0=%d, c1=%d, z1=%d\n",
//				i, SSSsubfr0[2*i], i, SSSsubfr5[2*i+1], s0, s1, c0, c1, z1);
//	}

	return 1;
}



int sign (float value){
	if(value==0.0)return 0;
	return (value>0.0)?1:(-1);
}

/**@ingroup getCellId
 * This module recovers the SSS sequence and identifies the phylayerCellID.
 * Obtain the subframe number and slot inside the LTE frame
 * \param PSSid:Indicate the number of PSS sequence.
 * \param phylayerCellID; Output: Physical-layer cell identity: 0-503
 * \param nofsubframe: Indicates the number of subframe in the LTE frame.
 * Return 1 if OK, -1 if error.
 */

int getCellId (float *SSSsubfr, int PSSid, int *m0s, int *m1s, synchctrl2_t *subframCtrl){

	int i, j, k, SSSid;
	int sum0, max0;
	int sum1, max1;
	int m0, m1;
	int ss0[N], ss1[N], cc0[N], cc1[N], zz0[N], zz1[N];
	int sm0[N], sm1[N], zm1[N];
	int Rxm0, Rxm1;

	for(i=0; i<N; i++){
		cc0[i] = cc[(i+PSSid)%N];
		cc1[i] = cc[(i+PSSid+3)%N];
	}
	//Check if Subframe0
	for (i=0; i<N; i++){
		sm0[i] = sign(SSSsubfr[2*i])*cc0[i];//c0Rx;
	}

//	printf("RX PSSid=%d\n", PSSid);
//	printf("sm0------------\n");
//	for(i=0; i<N; i++){
//		printf("sm0[i]=%d, cc0[i]=%d, SSSsubfr0[2*i]=%2.2f ss0[]=%d\n", sm0[i], cc0[i], SSSsubfr[2*i], ss0[i]);
//	}

	//Get m0 or m1
	max0=0;
	Rxm0=0;

	for (j=0; j<N; j++){
		m0 = j;
		for(k=0;k<N; k++){
			ss0[k]=ss[(k+m0)%N];
		}
		sum0=0;
		for(i=0; i<N; i++){
			sum0 += sm0[i]*ss0[i];
		}
		if(max0<sum0){
			max0=sum0;
			Rxm0=j;
		}
	}
	m0=Rxm0;
	for(i=0; i<N; i++){
		zz0[i] = zz[(i+m0%8)%N];
	}
	for (i=0; i<N; i++){
		zm1[i] = sign(SSSsubfr[2*i+1])*cc1[i]*zz0[i];
	}
	max1=0;
	Rxm1=0;
	for (j=0; j<N; j++){
		m1 = j;
		for(k=0;k<N; k++){
			ss1[k]=ss[(k+m1)%N];
		}
		sum1=0;
		for(i=0; i<N; i++){
			sum1 += zm1[i]*ss1[i];//s1;
		}
		if(max1<sum1){
			max1=sum1;
			Rxm1=j;
		}
	}
	m1=Rxm1;
	if(max0 < 31 || max1 < 31){
		modinfo("SSS sequence no correctly detected\n");
		return -1;
	}
	subframCtrl->nofsubframe=0;
	subframCtrl->nofslot=0;
	subframCtrl->nofOFDMsymb=7;
	if(Rxm0>Rxm1){
		m0=Rxm1;
		m1=Rxm0;
		subframCtrl->nofsubframe=5;
		subframCtrl->nofslot=10;
		subframCtrl->nofOFDMsymb=77;
	}
	SSSid = getIdFromTable (m0s, m1s, m0, m1);
	if(SSSid==-1){
		subframCtrl->phyLayerCellID = -1;
		return -1;
	}
	subframCtrl->phyLayerCellID = SSSid*3+PSSid;

	return 1;
}

/**@ingroup getCellId
 * This module recovers the SSS sequence and identifies the phylayerCellID.
 * Obtain the subframe number and slot inside the LTE frame
 * \param PSSid:Indicate the number of PSS sequence.
 * \param phylayerCellID; Output: Physical-layer cell identity: 0-503
 * \param nofsubframe: Indicates the number of subframe in the LTE frame.
 * Return 1 if OK, -1 if error.
 */
int detect_SSS (int FFTsize, _Complex float *OFDMsymbol, int PSSid, int *m0s, int *m1s, synchctrl2_t *subframCtrl){

	int i, j, s;
	_Complex float filteredIN[MAXFFTSIZE], *pOFDMsymbol;
	fftwf_plan plan128;
	_Complex float SSS[MAXFFTSIZE];
	float SSSfr[MAXFFTSIZE];
	int inputlength=FFTsize;
	int FIRlength, delayFIR;

	moddebug("subframCtrl->p2_OFDMSSS=%d\n", subframCtrl->p2_OFDMSSS);
	subframCtrl->p2_OFDMSSS=subframCtrl->p2_OFDMPSS - FFTsize;
	pOFDMsymbol = (_Complex float *)(OFDMsymbol + subframCtrl->p2_OFDMSSS);

	//Filter input to extract SSS
	if(FFTsize==128){
		FIRlength=31;
		delayFIR=FIRlength/2;
		PSS_LPrealfiltering(inputlength+FIRlength, pOFDMsymbol, filteredIN, LPPSS_128filter, FIRlength);
	}
	if(FFTsize==512){
		modinfo("ALL FILTERS SHOULD HAVE THE SAME LENGTH\n");
		PSS_LPrealfiltering(inputlength+2*31, OFDMsymbol, filteredIN, LPPSS_512filter, 33);
	}

	//FFT: Goto Frequency Domain
	plan128 = fftwf_plan_dft_1d(128, filteredIN+delayFIR, SSS, FFTW_FORWARD, FFTW_ESTIMATE);
	fftwf_execute(plan128);

	//Rotate Spectrum
	s=1;					//DC at position O
	for(i=SSSLEN/2; i<SSSLEN; i++){
		SSSfr[i] = __real__ SSS[s];
		s++;
	}
	s=(FFTsize-(SSSLEN/2));
	for(i=0; i<SSSLEN/2; i++){
		SSSfr[i] = __real__ SSS[s];
		s++;
	}
	//Recover Cell ID
	return getCellId (SSSfr, subframCtrl->PSSseq, m0s, m1s, subframCtrl);
}

/**@ingroup genPSStime_seq
 * This module generate the PSS time sequence for the different FFT size
 * \param phylayerCellID: Physical-layer cell identity: 0-503
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048
 * \param TxRxmode: defines if the sequence generate is for Tx or Rx side
 */
int genSSStime_seq(int phylayerCellID, int FFTsize, fftwf_complex *SSSsubfr0time, fftwf_complex *SSSsubfr5time){

	int s, j, i, k;
	int m0s[NUMSSS];
	int m1s[NUMSSS];

	float SSSsf0[SSSLEN+2];
	float SSSsf5[SSSLEN+2];
	float *pinSSS;

	fftwf_complex *pin, *pout;

	/**FFT*/
	fftwf_complex SSSsubfr0freq[2048], SSSsubfr5freq[2048];
	fftwf_plan plan64, plan128, plan256, plan512, plan1024, plan1536, plan2048;

	/***/
	loadmtable (m0s, m1s);
	setSSS(phylayerCellID, SSSsf0, SSSsf5, m0s, m1s);

	for(k=0; k<2; k++){
		if(k==0){
			pinSSS = SSSsf0;
			pin = SSSsubfr0freq;
			memset(pin, 0, sizeof(_Complex float)*FFTsize);
			pout = SSSsubfr0time;

		}
		if(k==1){
			pinSSS = SSSsf5;
			pin = SSSsubfr5freq;
			memset(pin, 0, sizeof(_Complex float)*FFTsize);
			pout = SSSsubfr5time;

		}
/*		for(i=0; i<128; i++){
			printf("ABANS ROTATE R_SSSsubfr0freq[%d]=%02.2f, I_SSSsubfr0freq[%d]=%02.2f, R_SSSsubfr5freq[%d]=%02.2f, I_SSSsubfr5freq[%d]=%02.2f\n",
					i, __real__ SSSsubfr0freq[i], i, __imag__ SSSsubfr0freq[i],
					i, __real__ SSSsubfr5freq[i], i, __imag__ SSSsubfr5freq[i]);
		}
*/
		//TX PSS: ROTATE
		//memset(pout, 0, sizeof(_Complex float)*FFTsize);
		s=1;	//DC at position O
		for(i=SSSLEN/2; i<SSSLEN; i++){
			__real__ pin[s] = pinSSS[i];
			s++;
		}
		s=(FFTsize-(SSSLEN/2));
		for(i=0; i<SSSLEN/2; i++){
			__real__ pin[s] = pinSSS[i];
			s++;
		}

/*		for(i=0; i<128; i++){
			printf("DESPRES ROTATE R_SSSsubfr0freq[%d]=%02.2f, I_SSSsubfr0freq[%d]=%02.2f, R_SSSsubfr5freq[%d]=%02.2f, I_SSSsubfr5freq[%d]=%02.2f\n",
				i, __real__ SSSsubfr0freq[i], i, __imag__ SSSsubfr0freq[i],
				i, __real__ SSSsubfr5freq[i], i, __imag__ SSSsubfr5freq[i]);
		}
*/
		if(FFTsize==64){
			plan64 = fftwf_plan_dft_1d(64, pin, pout, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan64);
			}
		if(FFTsize==128){
			plan128 = fftwf_plan_dft_1d(128, pin, pout, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan128);
		}
		if(FFTsize==512){
			plan512 = fftwf_plan_dft_1d(512, pin, pout, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan512);
		}
		if(FFTsize==1024){
			plan1024 = fftwf_plan_dft_1d(1024, pin, pout, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan1024);
		}
		if(FFTsize==1536){
			plan1536 = fftwf_plan_dft_1d(1536, pin, pout, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan1536);
		}
		if(FFTsize==2048){
			plan2048 = fftwf_plan_dft_1d(2048, pin, pout, FFTW_BACKWARD, FFTW_ESTIMATE);
			fftwf_execute(plan2048);
		}

	}
	return 0;
}


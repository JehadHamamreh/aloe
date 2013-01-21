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
#include <complex.h>
#define INCLUDE_DEFS_ONLY
#include "lte_resourMapp.h"
#include "syncSignals.h"

/** Global variables*/
_Complex float PSSsymb[PSSLEN];
float SSSseq[SSSLEN];

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
	if(phylayerID == 1)rootidx = PSSCELLID0;
	if(phylayerID == 2)rootidx = PSSCELLID0;


	for(i=0; i<PSSLEN/2; i++){
		arg=(float)TxRxMode*PI*rootidx*((float)i*((float)i+1.0))/63.0;
		__real__ PSSsymb[i]=cos(arg);
		__imag__ PSSsymb[i]=sin(arg);
	}
	for(i=PSSLEN/2; i<PSSLEN; i++){
		arg=(float)TxRxMode*PI*rootidx*(((float)i+2.0)*((float)i+1.0))/63.0;
		__real__ PSSsymb[i]=cos(arg);
		__imag__ PSSsymb[i]=sin(arg);
	}
	return 1;
}



/** SECONDARY SYNCH SIGNALS*/
#define N 31
#define qp (int)floor((float)id1/30.0)
#define q  (int)floor((float)(id1+qp*(qp+1)/2)/30.0)
#define m  (int)(id1 + q*(q+1)/2)
#define z0 z[(i+m0%8)%N]
#define z1 z[(i+m1%8)%N]
#define c0 c[(i+id2)%N]
#define c1 c[(i+id2+3)%N]
#define s0 s[(i+m0)%N]
#define s1 s[(i+m1)%N]

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

/**
 * @brief Function documentation: setSSS()
 * Algorithm taken from  3GPP TS 36.211 version 10.5.0 Release 10 Section 6.11.2
 * @params
 * @params int cellid:Physical Layer Cell-Identity (0-503)
 * @params float *SSSseq: SSS sequence.
 * @params int *m0s: mo index.
 * @params int *m1s: m1 index.
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int setSSS(int cellid, float *SSSseq, int *m0s, int *m1s)
{
	int i, id1 = cellid/3, id2 = cellid%3;
	int s[N], x[N], c[N], z[N];
	int m0 = m0s[id1];
	int m1 = m1s[id1];

	if(cellid >= MAXPHYLAYERCELLID)return -1;

	x[0] = 0;
	x[1] = 0;
	x[2] = 0;
	x[3] = 0;
	x[4] = 1;

	for (i=0; i<26; i++) x[i+5] = (x[i+2]+x[i])%2;
	for (i=0; i<N; i++) s[i] = 1-2*x[i];

	for (i=0; i<26; i++) x[i+5] = (x[i+3]+x[i])%2;
	for (i=0; i<N; i++) c[i] = 1-2*x[i];

	for (i=0; i<26; i++) x[i+5] = (x[i+4]+x[i+2]+x[i+1]+x[i])%2;
	for (i=0; i<N; i++) z[i] = 1-2*x[i];

	for (i=0; i<N; i++){
		/** Even Resource Elements: Sub-frame 0*/
		SSSseq[2*i] = (float)(s0*c0);
		/** Odd Resource Elements: Sub-frame 0*/
		SSSseq[2*i+1] = (float)(s1*c1*z0);
	}
	for (i=0; i<N; i++){
		/** Even Resource Elements: Sub-frame 5*/
		SSSseq[2*i+N*2]   = (float)(s1*c0);
		/** Odd Resource Elements: Sub-frame 5*/
		SSSseq[2*i+1+N*2] = (float)(s0*c1*z1);
	}

#ifdef 	PRINT_TEST
	for(i=0; i<4*N; i++){
		printf("SSSseq[%d]=%02.2f\n", i, SSSseq[i]);
	}
#endif

	return 1;

}


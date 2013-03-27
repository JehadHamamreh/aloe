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

#ifndef _SYNCSIGNALS_H
#define _SYNCSIGNALS_H

#define PI 3.14159265

#define MAXPHYLAYERCELLID 504	/** Max value for the Physical Layer Cell-ID*/

// Primary synchronization signal definitions
#define PSSLEN 		62	//Number of PSS Symbols in Zadoff-Chu sequence
#define PSSCELLID0 	25.0
#define PSSCELLID1 	29.0
#define PSSCELLID2 	34.0

// Secondary synchronization signal definitions
#define SSSLEN 		62	//Number of SSS Symbols
#define NUMSSS 		168
#define N 		31
//int m0s[NUMSSS];
//int m1s[NUMSSS];
float SSS_i [SSSLEN];
float SSS_q [SSSLEN];
float SSS   [SSSLEN];


typedef struct subframe2{
	int CPtype;
	int p2_subframe;	//point to 1st sample of subframe (including CPs)
	int nofsubframe;	//Current number of subframe in half LTE frame: 0 ..5
	int p2_OFDMPSS;		//Point to 1st sample of OFDM PSS symbol (including CP)
	int p2_OFDMSSS;		//Point to 1st sample of OFDM SSS symbol (Not including CP)
	int nofOFDMsymb;	//Current number of OFDM symb in half LTE frame: 0..NOFSYMBLTEFRAME/2
	int nofslot;		//Current number for slot in half LTE frame: 0..10
	int synchstate;		//Define the subframe detection state: INIT, TRACK
	int PSSseq;
	int phyLayerCellID;	//Detected PHY-Layer Cell ID
	int frametype;		//LTE frame type
}synchctrl2_t;

void initSSStables(int *z, int *s, int *c);
int setPSS(int CellID, _Complex float *PSSsymb, int TxRxMode);
int genPSStime_seq(int cellID, int FFTsize, _Complex float *PSS_time, int TxRxmode);
void loadmtable (int *m0s, int *m1s);
int setSSS(int cellid, float *SSSsubfr0, float *SSSsubfr5, int *m0s, int *m1s);
int sign (float value);
int getIdFromTable (int * m0s, int * m1s, int m0, int m1);
//int getCellId (float *SSSsubfr, int PSSid, int *m0s, int *m1s, int *phylayerCellID, int *nofsubframe);
int getCellId (float *SSSsubfr, int PSSid, int *m0s, int *m1s, synchctrl2_t *subframCtrl);
//int detect_SSS (int FFTsize, _Complex float *OFDMsymbol, int PSSid, int *m0s, int *m1s, int *phylayerCellID, int *nofsubframe);
int detect_SSS (int FFTsize, _Complex float *OFDMsymbol, int PSSid, int *m0s, int *m1s, synchctrl2_t *subframCtrl);
int genSSStime_seq(int phylayerCellID, int FFTsize, fftwf_complex *SSSsubfr0time, fftwf_complex *SSSsubfr5time);

#endif

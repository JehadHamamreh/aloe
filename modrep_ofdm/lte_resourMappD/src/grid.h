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

#ifndef _GRID_H
#define _GRID_H

/* Grid pattern identifiers */
#define REFSIGN	'R'	// Symbol for reference signals
#define SECSIGN 'S'	// Symbol for secondary signals
#define PRISIGN 'P'	// Symbol for primary signals
#define TRASIGN 'T'	// Symbol for traffic signals
#define ZERSIGN '_' // Symbol for no signal

/* LTE physical channels */
#define RESERV 'X' // Reserved for Reference signals not used
#define PDSCH  'D' // Physical Downlink Shared Channel
#define PBCH   'B' // Physical Broadcast Channel
#define PMCH   'M' // Physical Multicast Channel
#define PCFICH 'F' // Physical Control Format Indicator Channel
#define PDCCH  'C' // Physical Downlink Control Channel
#define PHICH  'H' // Physical Hybrid ARQ Indicator Channel

/* Grid functions definition */
int setGrid (int numOFDM, int numDL, int fftSize, int cellid, char * grid);
void printGrid (int numOFDM, int fftSize, char * grid);
void printGrid2 (int numOFDM, int fftSize, _Complex float * grid);			//ELIMINAR
int nofMSymbolsReq(char *OFDMgrid, int *nofSymbXFFT, int numOFDM, int fftsize);
int nofMSymbolxSLOT(int *nofSymbXFFT, int *nofSymbxSLOT, int numOFDM, int nofSLOTS);
int allocateDataOneUSer (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, input_t *in, output_t *gridSlot);
int allocateCRS (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, input_t *in, output_t *gridSlot);
int allocatePSS (char *OFDMgrid, int fftSize, int numSlot,\
		int numOFDMsymbxSlot, _Complex float *PSSsymbols, output_t *gridSlot);
int allocateSSS (char *OFDMgrid, int fftSize, int numSlot,\
		int numOFDMsymbxSlot, float *SSSsymbols, output_t *gridSlot);

#endif

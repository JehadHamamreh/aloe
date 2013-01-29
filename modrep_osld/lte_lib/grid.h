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


/** @defgroup lte_lib lte_lib
 *
 * Library of common LTE routines and constants
 *
 * @{
 */



/* Grid pattern identifiers */
#define REFSIGN	'R'	/* Symbol for reference signals */
#define SECSIGN 'S'	/* Symbol for secondary signals*/
#define PRISIGN 'P'	/* Symbol for primary signals*/
#define TRASIGN 'T'	/* Symbol for traffic signals*/
#define ZERSIGN '_' /* Symbol for no signal*/

/* LTE physical channels */
#define RESERV 'X' /* Reserved for Reference signals not used*/
#define PDSCH  'D' /* Physical Downlink Shared Channel*/
#define PBCH   'B' /* Physical Broadcast Channel*/
#define PMCH   'M' /* Physical Multicast Channel*/
#define PCFICH 'F' /* Physical Control Format Indicator Channel*/
#define PDCCH  'C' /* Physical Downlink Control Channel*/
#define PHICH  'H' /* Physical Hybrid ARQ Indicator Channel*/


#define LTE_NOF_SLOTS_X_FRAME	20

/*@} */

int lte_get_psdch_bits_x_slot(int slot_idx, int fft_size, int cp_is_long, int nof_slots);
int get_bits_per_symbol(int modulation);
int lte_get_modulation_format(int mcs);
int lte_get_tbs(int mcs, int nrb);
int lte_get_cbits(int mcs, int nrb);

/* Grid functions definition */
int setGrid (int numOFDM, int numDL, int fftSize, int cellid, char * grid);
void printGrid (int numOFDM, int fftSize, char * grid);
void printGrid2 (int numOFDM, int fftSize, _Complex float * grid);
int nofMSymbolsReq(char *OFDMgrid, int *nofSymbXFFT, int numOFDM, int fftsize);
int nofMSymbolxSLOT(int *nofSymbXFFT, int *nofSymbxSLOT, int numOFDM, int nofSLOTS);



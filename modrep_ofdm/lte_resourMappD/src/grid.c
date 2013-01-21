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
#include <math.h>
#include <complex.h>
#define INCLUDE_DEFS_ONLY
#include "lte_resourMapp.h"
#include "grid.h"

/* Special conditions for PCFICH (in MACROS) */
#define J ((j<fftSize/2)?(j-0):(j-1))
#define PCFICHCOND0 (J-downGuard>=(k)%(numsubC) && J-downGuard<(k)%numsubC+6)
#define PCFICHCOND1 (J-downGuard>=(k+(int)floor(1.0*(float)numPRBs/2.0)*12/2)%numsubC && J-downGuard<(k+(int)floor(1.0*(float)numPRBs/2.0)*12/2)%numsubC+6)
#define PCFICHCOND2 (J-downGuard>=(k+(int)floor(2.0*(float)numPRBs/2.0)*12/2)%numsubC && J-downGuard<(k+(int)floor(2.0*(float)numPRBs/2.0)*12/2)%numsubC+6)
#define PCFICHCOND3 (J-downGuard>=(k+(int)floor(3.0*(float)numPRBs/2.0)*12/2)%numsubC && J-downGuard<(k+(int)floor(3.0*(float)numPRBs/2.0)*12/2)%numsubC+6)

/**
 * @brief Function documentation
 * This function defines the type of information allocated at each Resource Element in the
 * LTE Frame.
 * @params
 * @params char *grid. Is a grid  where all the Resource Elements in the LTE frame Grid
 * has been tagged with the corresponding info type.
 * @params int numOFDM: Number of OFDM Symbols in a LTE frame.
 * @params int numsubC: Number of subcarriers.
 * @params int fftsize:
 * @params int cellid: A value between 0 and 503.
 *
 * Valid only for antenna port 0. To be extended to other ports.
 *
 * @return On success returns 1.
 * On error returns -1.
 */
int setGrid (int numOFDM, int numsubC, int fftSize, int cellid, char * grid){
	// Secondary needed parameters
	// drived from primmary
	int downGuard = (fftSize-numsubC-1)/2;
	int upGuard = fftSize-numsubC-downGuard-1;
	int v;
	int vshift = cellid%6;
	int ntraffic=0;
	int subframesym = 7;
	int numPRBs = (numsubC/12);
	int controlChannelLen = 3; // For LTE 1-3 for MBSFN transmitions and 2-4 for narrow bands (<10 RBs)
	int k = (12/2)*(cellid%(2*numPRBs));

	//indexes and grid definition
	int i, j=0;

	//Grid Generation
	for (i=0; i<numOFDM; i++){
		if (i%7==0) v=0;
		else v=3;
		if (j==fftSize/2-1) v++;

		for(j=0; j<fftSize; j++){
			if (j==fftSize/2-1)grid[i*fftSize+j]=ZERSIGN;
			else if (j<downGuard)grid[i*fftSize+j]=ZERSIGN;
			else if (j>(fftSize-upGuard-1))	grid[i*fftSize+j]=ZERSIGN;
			else if (( (j-downGuard-vshift-v)%6==0 ) && ( i%7==0 ))	grid[i*fftSize+j]=REFSIGN;
			else if (( (j-downGuard-vshift-v)%6==0 ) && ( (i-4)%7==0 ))	grid[i*fftSize+j]=REFSIGN;
			else if ((i==5 || i==75)&&(j<fftSize/2-1-31))grid[i*fftSize+j]=ZERSIGN;
			else if ((i==5 || i==75)&&(j>fftSize/2-1+31))grid[i*fftSize+j]=ZERSIGN;
			else if ( i==5 || i==75)grid[i*fftSize+j]=SECSIGN;
			else if ((i==6 || i==76)&&(j<fftSize/2-1-31))grid[i*fftSize+j]=ZERSIGN;
			else if ((i==6 || i==76)&&(j>fftSize/2-1+31))grid[i*fftSize+j]=ZERSIGN;
			else if ( i==6 || i==76)grid[i*fftSize+j]=PRISIGN;
			else if ((i>6)&&(i<11)&&(j>fftSize/2-36-2)&&(j<fftSize/2+36)){ // Broadcast region
				if (((j-downGuard-vshift-v)%6==0)&&((i-1)%7==0))grid[i*fftSize+j]=RESERV;
				else if (((j-downGuard-vshift-v+3)%6==0)&&((i-1)%7==0))	grid[i*fftSize+j]=RESERV;
				else if (((j-downGuard-vshift-v+3)%6==0)&&((i)%7==0))grid[i*fftSize+j]=RESERV;
				else grid[i*fftSize+j]=PBCH;
			}
			else if (i%(2*subframesym)==0){ // if PDCCH region
				if 		((j-downGuard-vshift-v+3)%6==0 )grid[i*fftSize+j]=RESERV;
				else if (PCFICHCOND0)grid[i*fftSize+j]=PCFICH;
				else if (PCFICHCOND1)grid[i*fftSize+j]=PCFICH;
				else if (PCFICHCOND2)grid[i*fftSize+j]=PCFICH;
				else if (PCFICHCOND3)grid[i*fftSize+j]=PCFICH;
				else grid[i*fftSize+j]=PDCCH;
			}
			else if (i%(2*subframesym)<controlChannelLen)grid[i*fftSize+j]=PDCCH;
			else {
				grid[i*fftSize+j]=TRASIGN;
				ntraffic++;
			}
			if (j==fftSize/2-1) v+=1;
		}
	}

	//return ntraffic;
	return fftSize*numOFDM;
}

/*
 * Print grid characters in console
 */
void printGrid (int numOFDM, int fftSize, char * grid)
{
	int i, j;

	printf("GRID NOTATION\n");
	printf("'R' =  Symbol for reference signals\n");
	printf("'S' =  Symbol for secondary synchro signals\n");
	printf("'P' =  Symbol for primary synchro signals\n");
	printf("'T' =  Symbol for data traffic signals\n");
	printf("'_' =  Symbol for no signal\n");
	printf("'X' =  Reserved for Reference signals. Not used\n");
	printf("'B' =  Physical Broadcast Channel\n");
	printf("'F' =  Physical Control Format Indicator Channel\n");
	printf("'C' =  Physical Downlink Control Channel\n");

	printf("\n");
	for (i=0; i<numOFDM; i++){
		printf("[%.3d] ", i);
		for(j=0; j<fftSize; j++){
			printf("%c", grid[i*fftSize+j]);
		}
		printf("\n");
	}
	printf("GRID NOTATION\n");
	printf("'R' =  Symbol for reference signals\n");
	printf("'S' =  Symbol for secondary synchro signals\n");
	printf("'P' =  Symbol for primary synchro signals\n");
	printf("'T' =  Symbol for data traffic signals\n");
	printf("'_' =  Symbol for no signal\n");
	printf("'X' =  Reserved for Reference signals. Not used\n");
	printf("'B' =  Physical Broadcast Channel\n");
	printf("'F' =  Physical Control Format Indicator Channel\n");
	printf("'C' =  Physical Downlink Control Channel\n");
}
/*
 * To visualize the correct allocation of data (data modified to adjust the format)
 */

void printGrid2 (int numOFDM, int fftSize, _Complex float * grid)	//ELIMINAR
{
	int i, j;
	int m;

	printf("\n");
	for (i=0; i<numOFDM; i++){
		printf("[%.3d] ", i);
		for(j=0; j<fftSize; j++){
			m=(int) __real__ grid[i*fftSize+j];
			m = m%9;
			if(m < 0)m = 0;
			printf("%1d", m);
		}
		printf("\n");
	}
}

/**
 * @brief Function documentation
 * This function performs the calculation of the required modulated (4QAM, 16QAM, 64QAM) symbols
 * at each OFDM symbol in the LTE Downlink frame.
 * Full RB allocation to one user. No partial RB allocation is considered.
 * @params
 * @params char *OFDMgrid. Is the grid defined by setGrid() function where all the Resource
 * Elements in the LTE frame Grid has been tagged with the corresponding info type.
 * @params int numOFDM: Number of OFDM Symbols in a LTE frame.
 * @params int fftsize:
 *
 * @return On success returns 1.
 * On error returns -1.
 */
int nofMSymbolsReq(char *OFDMgrid, int *nofSymbXFFT, int numOFDM, int fftsize){

	int i, j;

	for(i=0; i<numOFDM; i++){
			nofSymbXFFT[i]=0;
		}
	for (i=0; i<numOFDM; i++){
		for(j=0; j<fftsize; j++){
			if(OFDMgrid[i*fftsize+j] == 'T')nofSymbXFFT[i]=nofSymbXFFT[i]+1;
		}
	}
	return (1);
}
/**
 * @brief Function documentation:
 * This function calculates the number of MQAM symbols required by the resource mapper
 * at each slot (0.5 ms) and fill the nofSymbxSLOT array.
 * Full RB allocation to one user. No partial RB allocation is considered.
 * @params int *nofSymbXFFT. Is the grid defined by nofMSymbolsReq() function where all
 * the number of MQAM Symbols has been calculated for each OFDM Symbol in the LTE frame.
 * @params int numOFDM: Number of OFDM Symbols in a LTE frame.
 * @params int numSLOTS: Number of Slots in a LTE frame.
 *
 * @return On success, returns a non-negative number indicating the number of data symbols
 * allocated..
 * On error returns -1.
 */

int nofMSymbolxSLOT(int *nofSymbXFFT, int *nofSymbxSLOT, int numOFDM, int nofSLOTS){

	int i, j, s, value, nofFFTxSLOT;

	//printf("numOFDM=%d nofSLOTS=%d\n", numOFDM,nofSLOTS);

	nofFFTxSLOT=numOFDM/20;

	for(i=0; i<nofSLOTS; i++){
		nofSymbxSLOT[i]=0;
		}

	value=0;
	j=0;
	s=0;
	for (i=0; i<numOFDM; i++){
		value=value+nofSymbXFFT[i];
		s++;
		if(s == nofFFTxSLOT){
			s=0;
			nofSymbxSLOT[j]=value;
			//printf("nofSymbxSLOT[%d]=%d\n", j, nofSymbxSLOT[j]);
			j++;
			value=0;
		}
	}
	return (1);
}

/**
 * @brief Function documentation
 * This function allocates the data symbols (MQAM) received in a slot (0.5 ms)from previous
 * module to the available resources. All the available resources are assigned for one user.
 * No partial RB allocation is done.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot: Slot number in the LTE frame (0-19).
 * @param output_t *gridSlot: .
 *
 * @return On success, returns a non-negative number indicating the number of data symbols
 * allocated..
 * On error returns -1.
 */

int allocateDataOneUser (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, input_t *in, output_t *gridSlot)
{
	int i, j, s, numOFDM;

	numOFDM=numSlot*numsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'T'){
				gridSlot[i*fftSize+j]=  *(in+s);
				s++;
			}
		}
	}
	return s;
}

/**
 * @brief Function documentation
 * This function allocates Cell-Specific Reference Signals (CRS) to the assigned resources
 * in one slot of the LTE frame.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot: Slot number in the LTE frame (0-19).
 * @params int numOFDMsymbxSlot: Number of OFDM symbols per slot.
 * @params _Complex float *CRSvalues: Input Array of CRS values.
 * @param output_t *gridSlot: Output LTE grid with CRS incorporated.
 *
 * @return On success, returns a non-negative number indicating the number of CRS symbols
 * allocated.
 * On error returns -1.
 */

int allocateCRS (char *OFDMgrid, int fftSize,\
		int numSlot, int numOFDMsymbxSlot, _Complex float *CRSvalues, output_t *gridSlot)
{
	int i, j, s, numOFDM;

	numOFDM=numSlot*numOFDMsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numOFDMsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'R'){
				gridSlot[i*fftSize+j]= *(CRSvalues+s);
				s++;
			}
		}
	}
	return s;
}

/**
 * @brief Function documentation
 * This function allocates Cell-Specific Reference Signals (CRS) to the assigned resources
 * in the slot of the LTE frame.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot:
 * @params int numOFDMsymbxSlot: Number of OFDM symbols per slot.
 * @params _Complex float *CRSvalues: Input Array of CRS values.
 * @param output_t *gridSlot: Output LTE grid with CRS incorporated.
 *
 * @return On success, returns a non-negative number indicating the number of CRS symbols
 * allocated.
 * On error returns -1.
 */

int allocatePSS (char *OFDMgrid, int fftSize,\
		int numSlot, int numOFDMsymbxSlot, _Complex float *PSSsymbols, output_t *gridSlot)
{
	int i, j, s, numOFDM;

	numOFDM=numSlot*numOFDMsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numOFDMsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'P'){
				gridSlot[i*fftSize+j] = *(PSSsymbols+s);
				s++;
			}
		}
	}
	return s;
}

/**
 * @brief Function documentation
 * This function allocates Cell-Specific Reference Signals (CRS) to the assigned resources
 * in the slot of the LTE frame.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot:
 * @params int numOFDMsymbxSlot: Number of OFDM symbols per slot.
 * @params _Complex float *CRSvalues: Input Array of CRS values.
 * @param output_t *gridSlot: Output LTE grid with CRS incorporated.
 *
 * @return On success, returns a non-negative number indicating the number of CRS symbols
 * allocated.
 * On error returns -1.
 */

int allocateSSS (char *OFDMgrid, int fftSize, int numSlot,\
		int numOFDMsymbxSlot, float *SSSsymbols, output_t *gridSlot){
	int i, j, s, numOFDM;

	numOFDM=numSlot*numOFDMsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numOFDMsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'S'){
				if(numSlot==0){
					/** Frame Type 1*/
					__real__ gridSlot[i*fftSize+j]= __real__ *(SSSsymbols+s);
					__imag__ gridSlot[i*fftSize+j]= __imag__ *(SSSsymbols+s);
				}
				if(numSlot==1){
					/** Frame Type 2*/
					printf("FRAME TYPE 2: TO BE INCORPORATED\n");
				}
				if(numSlot==10){
					/** Frame Type 1*/
					__real__ gridSlot[i*fftSize+j]= __real__ *(SSSsymbols+s+62);
					__imag__ gridSlot[i*fftSize+j]= __imag__ *(SSSsymbols+s+62);
				}
				if(numSlot==11){
					/** Frame Type 2*/
					printf("FRAME TYPE 2: TO BE INCORPORATED\n");
				}
				s++;
			}
		}
	}
	return s;
}

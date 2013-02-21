#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include "allocate_signals.h"

/**
 *
 * This function allocates the data symbols (MQAM) received in a slot (0.5 ms)from previous
 * module to the available resources. All the available resources are assigned for one user.
 * No partial RB allocation is done.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot: Slot number in the LTE frame (0-19).
 * @param _Complex float *gridSlot: .
 *
 * @return On success, returns a non-negative number indicating the number of data symbols
 * allocated..
 * On error returns -1.
 */

int allocateDataOneUser (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, _Complex float *in, _Complex float *gridSlot)
{
	int i, j, s, numOFDM;

	numOFDM=numSlot*numsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'T'){
				gridSlot[i*fftSize+j]=  *(in+s);
				s++;
			} else if (OFDMgrid[i*fftSize+j] == 'B') {
				gridSlot[i*fftSize+j] = rand()/RAND_MAX;//1.0+_Complex_I*1.0;
			}else if (OFDMgrid[i*fftSize+j] == 'C') {
				gridSlot[i*fftSize+j] = rand()/RAND_MAX;//1.0+_Complex_I*1.0;
			}else if (OFDMgrid[i*fftSize+j] == 'F') {
				gridSlot[i*fftSize+j] = rand()/RAND_MAX;//1.0+_Complex_I*1.0;
			}

		}
	}
	return s;
}

/**
 *
 * This function allocates Cell-Specific Reference Signals (CRS) to the assigned resources
 * in one slot of the LTE frame.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot: Slot number in the LTE frame (0-19).
 * @params int numOFDMsymbxSlot: Number of OFDM symbols per slot.
 * @params _Complex float *CRSvalues: Input Array of CRS values.
 * @param _Complex float *gridSlot: Output LTE grid with CRS incorporated.
 *
 * @return On success, returns a non-negative number indicating the number of CRS symbols
 * allocated.
 * On error returns -1.
 */

int allocateCRS (char *OFDMgrid, int fftSize,\
		int numSlot, int numOFDMsymbxSlot, _Complex float *CRSvalues, _Complex float *gridSlot)
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
 *
 * This function allocates the data symbols (MQAM) received in a slot (0.5 ms)from previous
 * module to the available resources. All the available resources are assigned for one user.
 * No partial RB allocation is done.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot: Slot number in the LTE frame (0-19).
 * @param _Complex float *gridSlot: .
 *
 * @return On success, returns a non-negative number indicating the number of data symbols
 * allocated..
 * On error returns -1.
 */

int deallocateDataOneUser (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, _Complex float *input, _Complex float *output)
{
	int i, j, s, numOFDM;

	numOFDM=numSlot*numsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'T'){
				output[s] = input[(i-numOFDM)*fftSize+j];
				s++;
			}
		}
	}
	return s;
}

int deallocateCRS (char *OFDMgrid, int fftSize,\
		int numSlot, int numOFDMsymbxSlot, _Complex float *input, _Complex float *output)
{
	int i, j, s, numOFDM;

	numOFDM=numSlot*numOFDMsymbxSlot;

	s=0;
	for (i=numOFDM; i<numOFDM+numOFDMsymbxSlot; i++){
		for(j=0; j<fftSize; j++){
			if(OFDMgrid[i*fftSize+j] == 'R'){
				output[s] = input[(i-numOFDM)*fftSize+j];
				s++;
			}
		}
	}
	return s;
}

/**
 *
 * This function allocates Cell-Specific Reference Signals (CRS) to the assigned resources
 * in the slot of the LTE frame.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot:
 * @params int numOFDMsymbxSlot: Number of OFDM symbols per slot.
 * @params _Complex float *CRSvalues: Input Array of CRS values.
 * @param _Complex float *gridSlot: Output LTE grid with CRS incorporated.
 *
 * @return On success, returns a non-negative number indicating the number of CRS symbols
 * allocated.
 * On error returns -1.
 */

int allocatePSS (char *OFDMgrid, int fftSize,\
		int numSlot, int numOFDMsymbxSlot, _Complex float *PSSsymbols, _Complex float *gridSlot)
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
 *
 * This function allocates Cell-Specific Reference Signals (CRS) to the assigned resources
 * in the slot of the LTE frame.
 * @params char *OFDMgrid. Is the grid defined by setGrid() function.
 * @params int fftSize:
 * @params int numSlot:
 * @params int numOFDMsymbxSlot: Number of OFDM symbols per slot.
 * @params _Complex float *CRSvalues: Input Array of CRS values.
 * @param _Complex float *gridSlot: Output LTE grid with CRS incorporated.
 *
 * @return On success, returns a non-negative number indicating the number of CRS symbols
 * allocated.
 * On error returns -1.
 */

int allocateSSS (char *OFDMgrid, int fftSize, int numSlot,
		int numOFDMsymbxSlot, float *SSSsymbols, _Complex float *gridSlot){
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

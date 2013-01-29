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
#include <math.h>
#include <complex.h>

#include "refSignals.h"


/**
 *
 * This function calculates the Cell-Specific Reference Signals (CRS) to be incorporated to
 * a LTE frame.
 * @params
 * @params char *grid. Is a grid  where all the Resource Elements in the LTE frame Grid
 * has been tagged with the corresponding info type.
 * @params int numOFDM: Number of OFDM Symbols in a LTE frame.
 * @params int numsubC: Number of subcarriers.
 * @params int fftsize:
 * @params int phyLayerCellID: Values between 0-503.
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int setCRefSignals (int numOFDM, int numDL, int phyLayerCellID, _Complex float *CRS)
{
	int num_RS = numDL/6;
	int i, cinit, ns, l;
	int cyclic = 1;
	int x1 [GOLDMAXLEN];
	int x2 [GOLDMAXLEN];

	memset(x1, 0, sizeof(int)*GOLDMAXLEN);
	memset(x2, 0, sizeof(int)*GOLDMAXLEN);


	for (ns=0; ns<NUMSLOTS; ns++){
		l=0;
		cinit = Cinit;

		for (i=0; i<GOLDINITLEN; i++){
				x1[i] = (i==0)? 1:0;
				x2[i] = (int) (cinit & 0x1);
				cinit >>= 1;
		}


		for (i=0; i<GOLDMAXLEN-31; i++){
			x1[i+31] = (x1[i+3]+x1[i])%2;
			x2[i+31] = (x2[i+3]+x2[i+1]+x2[i+1]+x2[i])%2;
		}


		for (i=0; i<num_RS; i++){
			__real__ CRS[i+ns*num_RS*2]= 0.707107*(float)(1-2*(int)Cn(2*i));
			__imag__ CRS[i+ns*num_RS*2]= 0.707107*(float)(1-2*(int)Cn(2*i+1));
		}

		l=4;
		cinit = Cinit;

		for (i=0; i<GOLDINITLEN; i++){
				x1[i] = (i==0)? 1:0;
				x2[i] = (int) (cinit & 0x1);
				cinit >>= 1;
		}


		for (i=0; i<GOLDMAXLEN-31; i++){
			x1[i+31] = (x1[i+3]+x1[i])%2;
			x2[i+31] = (x2[i+3]+x2[i+1]+x2[i+1]+x2[i])%2;
		}


		for (i=0; i<num_RS; i++){
			__real__ CRS[i+(2*ns+1)*num_RS] = 0.707107*(float)(1-2*(int)Cn(2*i));
			__imag__ CRS[i+(2*ns+1)*num_RS] = 0.707107*(float)(1-2*(int)Cn(2*i+1));
		}

	}

	return num_RS*NUMSLOTS*2;
}



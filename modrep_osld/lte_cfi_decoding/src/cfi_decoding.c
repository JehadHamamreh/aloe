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

#include <stdio.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#define INCLUDE_DEFS_ONLY
#include "cfi_decoding.h"



/**
 * @ingroup lte_cfi_decoding
 * Decoding function
 * Decodes the recevied bitsequence to obtain the most likely CFI
 * The function performs bitwise XOR with each ot the coding sequences and
 * sums up the output. The minimum results indicates the code index and CFI.
 *
 * @param in Input bit sequence, received from descrambler, 32 bits
 * @param table Coding table, 4x32 bits
 * \returns Coding table index most similar to the recevied input sequence
 */
int cfi_decoding(char *in, char (*table)[NOF_BITS]) {

	int i, j;
	int count_i, index;
	int min = 32;

	for (i=0;i<4;i++) {
		count_i = 0;
		for (j=0;j<NOF_BITS;j++) {
			if (in[j] != table[i][j]) {
				count_i++;
			}
		}
		if (count_i < min) {
			min = count_i;
			index = i;
		}
	}
	return index;
}

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

#include <math.h>
#include <complex.h>

#define INCLUDE_DEFS_ONLY
#include "gen_soft_demod.h"

#include "soft_demod.h"


/**
 * @ingroup template
 * Calls the different modulation tables to be set
 *
 * \param table Pointer to all constellation tables
 * \param Sx Pointer to soft-demodulation auxiliary matrices
 *
 */
/*inline void set_modulation_tables(struct constellation_tables *table, struct Sx *Sx) {
	set_BPSKtable(table.bpsk, Sx.bpsk);
	set_QPSKtable(table.qpsk, Sx.qpsk);
	set_16QAMtable(table.qam16, Sx.qam16);
	set_64QAMtable(table.qam64, Sx.qam64);
}*/


/**
 * @ingroup template
 * Set the BPSK modulation table
 *
 * \param bpsk_table Pointer to the BPSK constellation table
 * \param Sx Pointer to soft-demodulation auxiliary matrices
 *
 */
inline void set_BPSKtable(_Complex float *bpsk_table, int (*S)[6][32]) {
	int i,j;
	/* LTE-BPSK constellation:
	    Q
	    |  0
	---------> I
	 1  |	  */
	bpsk_table[0] = BPSK_LEVEL + BPSK_LEVEL*_Complex_I;
	bpsk_table[1] = -BPSK_LEVEL -BPSK_LEVEL*_Complex_I;

	for (i=0;i<6;i++) {
		for (j=0;j<32;j++) {
			S[0][i][j] = 0;
			S[1][i][j] = 0;
		}
	}
	/* BSPK symbols containing a '0' and a '1' (only two symbols, 1 bit) */
	S[0][0][0] = 0;
	S[1][0][0] = 1;
}

/**
 * Set the QPSK modulation table */
inline void set_QPSKtable(_Complex float *qpsk_table, int (*S)[6][32]) {
	int i,j;
	/* LTE-QPSK constellation:
	     Q
	 10  |  00
	-----------> I
	 11  |  01 */
	qpsk_table[0] = QPSK_LEVEL + QPSK_LEVEL*_Complex_I;
	qpsk_table[1] = QPSK_LEVEL - QPSK_LEVEL*_Complex_I;
	qpsk_table[2] = -QPSK_LEVEL + QPSK_LEVEL*_Complex_I;
	qpsk_table[3] = -QPSK_LEVEL - QPSK_LEVEL*_Complex_I;

	for (i=0;i<6;i++) {
		for (j=0;j<32;j++) {
			S[0][i][j] = 0;
			S[1][i][j] = 0;
		}
	}

	/* QSPK symbols containing a '0' at the different bit positions */
	S[0][0][0] = 0;
	S[0][0][1] = 1;
	S[0][1][0] = 0;
	S[0][1][1] = 2;
	/* QSPK symbols containing a '1' at the different bit positions */
	S[1][0][0] = 2;
	S[1][0][1] = 3;
	S[1][1][0] = 1;
	S[1][1][1] = 3;
}

/**
 * Set the 16QAM modulation table */
inline void set_16QAMtable(_Complex float *qam16_table, int (*S)[6][32]) {
	int i,j;
	/* LTE-16QAM constellation:
	              Q
	  1011	1001  |	 0001  0011
	  1010	1000  |	 0000  0010
	---------------------------------> I
	  1110  1100  |  0100  0110
	  1111  1101  |  0101  0111 	*/
	qam16_table[0] = QAM16_LEVEL_1 + QAM16_LEVEL_1*_Complex_I;
	qam16_table[1] = QAM16_LEVEL_1 + QAM16_LEVEL_2*_Complex_I;
	qam16_table[2] = QAM16_LEVEL_2 + QAM16_LEVEL_1*_Complex_I;
	qam16_table[3] = QAM16_LEVEL_2 + QAM16_LEVEL_2*_Complex_I;
	qam16_table[4] = QAM16_LEVEL_1 - QAM16_LEVEL_1*_Complex_I;
	qam16_table[5] = QAM16_LEVEL_1 - QAM16_LEVEL_2*_Complex_I;
	qam16_table[6] = QAM16_LEVEL_2 - QAM16_LEVEL_1*_Complex_I;
	qam16_table[7] = QAM16_LEVEL_2 - QAM16_LEVEL_2*_Complex_I;
	qam16_table[8] = -QAM16_LEVEL_1 + QAM16_LEVEL_1*_Complex_I;
	qam16_table[9] = -QAM16_LEVEL_1 + QAM16_LEVEL_2*_Complex_I;
	qam16_table[10] = -QAM16_LEVEL_2 + QAM16_LEVEL_1*_Complex_I;
	qam16_table[11] = -QAM16_LEVEL_2 + QAM16_LEVEL_2*_Complex_I;
	qam16_table[12] = -QAM16_LEVEL_1 - QAM16_LEVEL_1*_Complex_I;
	qam16_table[13] = -QAM16_LEVEL_1 - QAM16_LEVEL_2*_Complex_I;
	qam16_table[14] = -QAM16_LEVEL_2 - QAM16_LEVEL_1*_Complex_I;
	qam16_table[15] = -QAM16_LEVEL_2 - QAM16_LEVEL_2*_Complex_I;

	for (i=0;i<6;i++) {
		for (j=0;j<32;j++) {
			S[0][i][j] = 0;
			S[1][i][j] = 0;
		}
	}

	/* Matrices identifying the zeros and ones of LTE-16QAM constellation */
	for (i=0;i<8;i++) {
		S[0][0][i] = i;   /* symbols with a '0' at the bit0 (leftmost)*/
		S[1][0][i] = i+8; /* symbols with a '1' at the bit0 (leftmost)*/
	}
	/* symbols with a '0' ans '1' at the bit position 1: */
	for (i=0;i<4;i++) {
		S[0][1][i] = i;
		S[0][1][i+4] = i+8;
		S[1][1][i] = i+4;
		S[1][1][i+4] = i+12;
	}
	/* symbols with a '0' ans '1' at the bit position 2: */
	for (j=0;j<4;j++) {
		for (i=0;i<2;i++) {
			S[0][2][i+2*j] = i + 4*j;
			S[1][2][i+2*j] = i+2 + 4*j;
		}
	}
	/* symbols with a '0' ans '1' at the bit position 3: */
	for (i=0;i<8;i++) {
		S[0][3][i] = 2*i;
		S[1][3][i] = 2*i+1;
	}
}

/**
 * Set the 64QAM modulation table */
inline void set_64QAMtable(_Complex float *qam64_table, int (*S)[6][32]) {
	int i, j;
	/* LTE-64QAM constellation:
                                         	   Q
	47-101111  45-101101  37-100101  39-100111 |  7-000111   5-000101  13-001101  15-001111
	46-101110  44-101100  36-100100  38-100110 |  6-000110   4-000100  12-001100  14-001110
	42-101010  40-101000  32-100000  34-100010 |  2-000010	 0-000000   8-001000  10-001010
	43-101011  41-101001  33-100001  35-100011 |  3-000011	 1-000001   9-001001  11-001011
	-------------------------------------------------------------------------------------------------------------> I
	59-111011  57-111001  49-110001  51-110011 | 19-010011  17-010001  25-011001  27-011011
	58-111010  56-111000  48-110000  50-110010 | 18-010010  16-010000  24-011000  26-011010
	62-111110  60-111100  52-110100  54-110110 | 22-010110	20-010100  28-011100  30-011110
	63-111111  61-111101  53-110101  55-110111 | 23-010111	21-010101  29-011101  31-011111  */

	qam64_table[0] = QAM64_LEVEL_2 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[1] = QAM64_LEVEL_2 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[2] = QAM64_LEVEL_1 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[3] = QAM64_LEVEL_1 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[4] = QAM64_LEVEL_2 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[5] = QAM64_LEVEL_2 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[6] = QAM64_LEVEL_1 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[7] = QAM64_LEVEL_1 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[8] = QAM64_LEVEL_3 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[9] = QAM64_LEVEL_3 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[10] = QAM64_LEVEL_4 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[11] = QAM64_LEVEL_4 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[12] = QAM64_LEVEL_3 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[13] = QAM64_LEVEL_3 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[14] = QAM64_LEVEL_4 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[15] = QAM64_LEVEL_4 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[16] = QAM64_LEVEL_2 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[17] = QAM64_LEVEL_2 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[18] = QAM64_LEVEL_1 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[19] = QAM64_LEVEL_1 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[20] = QAM64_LEVEL_2 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[21] = QAM64_LEVEL_2 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[22] = QAM64_LEVEL_1 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[23] = QAM64_LEVEL_1 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[24] = QAM64_LEVEL_3 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[25] = QAM64_LEVEL_3 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[26] = QAM64_LEVEL_4 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[27] = QAM64_LEVEL_4 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[28] = QAM64_LEVEL_3 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[29] = QAM64_LEVEL_3 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[30] = QAM64_LEVEL_4 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[31] = QAM64_LEVEL_4 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[32] = -QAM64_LEVEL_2 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[33] = -QAM64_LEVEL_2 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[34] = -QAM64_LEVEL_1 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[35] = -QAM64_LEVEL_1 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[36] = -QAM64_LEVEL_2 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[37] = -QAM64_LEVEL_2 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[38] = -QAM64_LEVEL_1 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[39] = -QAM64_LEVEL_1 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[40] = -QAM64_LEVEL_3 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[41] = -QAM64_LEVEL_3 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[42] = -QAM64_LEVEL_4 + QAM64_LEVEL_2*_Complex_I;
	qam64_table[43] = -QAM64_LEVEL_4 + QAM64_LEVEL_1*_Complex_I;
	qam64_table[44] = -QAM64_LEVEL_3 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[45] = -QAM64_LEVEL_3 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[46] = -QAM64_LEVEL_4 + QAM64_LEVEL_3*_Complex_I;
	qam64_table[47] = -QAM64_LEVEL_4 + QAM64_LEVEL_4*_Complex_I;
	qam64_table[48] = -QAM64_LEVEL_2 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[49] = -QAM64_LEVEL_2 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[50] = -QAM64_LEVEL_1 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[51] = -QAM64_LEVEL_1 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[52] = -QAM64_LEVEL_2 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[53] = -QAM64_LEVEL_2 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[54] = -QAM64_LEVEL_1 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[55] = -QAM64_LEVEL_1 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[56] = -QAM64_LEVEL_3 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[57] = -QAM64_LEVEL_3 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[58] = -QAM64_LEVEL_4 - QAM64_LEVEL_2*_Complex_I;
	qam64_table[59] = -QAM64_LEVEL_4 - QAM64_LEVEL_1*_Complex_I;
	qam64_table[60] = -QAM64_LEVEL_3 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[61] = -QAM64_LEVEL_3 - QAM64_LEVEL_4*_Complex_I;
	qam64_table[62] = -QAM64_LEVEL_4 - QAM64_LEVEL_3*_Complex_I;
	qam64_table[63] = -QAM64_LEVEL_4 - QAM64_LEVEL_4*_Complex_I;

	/* Matrices identifying the zeros and ones of LTE-64QAM constellation */

	for (i=0;i<32;i++) {
		S[0][0][i] = i;	/* symbols with a '0' at the bit0 (leftmost)*/
		S[1][0][i] = i+32;	/* symbols with a '1' at the bit0 (leftmost)*/
	}
	/* symbols with a '0' ans '1' at the bit position 1: */
	for (i=0;i<16;i++) {
		S[0][1][i] = i;
		S[0][1][i+16] = i+32;
		S[1][1][i] = i+16;
		S[1][1][i+16] = i+48;
	}
	/* symbols with a '0' ans '1' at the bit position 2: */
	for (i=0;i<8;i++) {
		S[0][2][i] = i;
		S[0][2][i+8] = i+16;
		S[0][2][i+16] = i+32;
		S[0][2][i+24] = i+48;
		S[1][2][i] = i+8;
		S[1][2][i+8] = i+24;
		S[1][2][i+16] = i+40;
		S[1][2][i+24] = i+56;
	}
	/* symbols with a '0' ans '1' at the bit position 3: */
	for (j=0;j<8;j++) {
		for (i=0;i<4;i++) {
			S[0][3][i+4*j] = i + 8*j;
			S[1][3][i+4*j] = i+4 + 8*j;
		}
	}
	/* symbols with a '0' ans '1' at the bit position 4: */
	for (j=0;j<16;j++) {
		for (i=0;i<2;i++) {
			S[0][4][i+2*j] = i + 4*j;
			S[1][4][i+2*j] = i+2 + 4*j;
		}
	}
	/* symbols with a '0' ans '1' at the bit position 5: */
	for (i=0;i<32;i++) {
		S[0][5][i] = 2*i;
		S[1][5][i] = 2*i+1;
	}
}

/**
 * Returns the number of bits per symbol
 * @param modulation modulation type */
inline int get_bits_per_symbol(int modulation) {

	switch(modulation) {
		case BPSK:
			return 1;
		case QPSK:
			return 2;
		case QAM16:
			return 4;
		case QAM64:
			return 6;
		default:
			moderror_msg("Unknown modulation %d\n",modulation);
			return -1;
	}
}


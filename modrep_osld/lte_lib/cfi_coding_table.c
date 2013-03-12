
#include "cfi_coding_table.h"

/**
 * @ingroup lte_cfi_decoding
 * Sets the coding table
 * This table contains four 32-bit sequences, one for each CFI plus one,
 * the last, being reserved
 * @param table Coding table, 4x32 bits
 *
 */
void coding_table(char (*table)[NOF_BITS]) {

	int i, j;

	/* Initialize first 3 seqeunces with all ones */
	for (i=0;i<3;i++) {
		for (j=0;j<NOF_BITS;j++) {
			table[i][j] = 0x1;
		}
	}
	/* Now set to zero the zero-bits */
	for (j=0;j<11;j++) {	/* 11 zeros @ bit position 0, 3, 6, ..., 30*/
		table[0][3*j] = 0x0;
	}
	for (j=0;j<11;j++) {	/* 11 zeros @ bit position 1, 4, 7, ..., 31*/
		table[1][3*j+1] = 0x0;
	}
	for (j=0;j<10;j++) {	/* 10 zeros @ bit position 2, 5, 8, ..., 29*/
		table[2][3*j+2] = 0x0;
	}
	/* 4th sequence--all zeros--is reserved */
	for (j=0;j<NOF_BITS;j++) {
		table[3][j] = 0x0;
	}
}

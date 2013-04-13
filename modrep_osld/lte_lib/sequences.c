#include <stdlib.h>
#include "sequences.h"

#define Nc 1600
#define GOLDMAXLEN (Nc*2)
static int x1 [GOLDMAXLEN];
static int x2 [GOLDMAXLEN];

/*
 * Pseudo Random Sequence generation.
 * It follows the 3GPP Release 8 (LTE) 36.211
 * Section 7.2
 */
void generate_prs_c (unsigned int seed,	int len, unsigned int* c) {
	int n;

	for(n = 0; n < 31; n++) {
		x1 [n] = 0;
		x2 [n] = (seed >> n) & 0x1;
	}
	x1 [0] = 1;

	for (n = 0; n < Nc + len; n++) {
		x1[n+31] = (x1[n+3] + x1[n]) & 0x1;
		x2[n+31] = (x2[n+3] + x2[n+2] + x2[n]) & 0x1;
	}

	for (n=0; n<len; n++) {
		c[n] = (x1[n+Nc] + x2[n+Nc]) & 0x1;
	}

}


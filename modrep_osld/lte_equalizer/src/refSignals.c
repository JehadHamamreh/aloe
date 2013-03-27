/*
 * refSignals.c
 *
 *  Created on: Feb 25, 2012
 *      Author: Xavier Arteaga
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "refSignals.h"


/* Maximum expected reference signals in the frame */
#define Ndlrbmax	110		// maximum number of downlink PRBs
#define Nc 1600
#define GOLDMAXLEN (Nc*2)
#define SQ2 1.41421356237f
#define NUMSLOTS 20

/*
 * Pseudo Random Sequence generation.
 * It follows the 3GPP Release 8 (LTE) 36<.211
 * Section 7.2
 */
void goldGen (	pn_t*			c,		// Sequence pointer
				unsigned int 	seed,	// Seed
				int 			len		// Sequence length
			){
	pn_t x1 [GOLDMAXLEN];
	pn_t x2 [GOLDMAXLEN];
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

/*
 * Generate the reference-signal sequence rl,ns (m).
 * It follows the 3GPP Release 8 (LTE) 36.211
 * Section 6.10.1.1
 */
void genRSSequence (		rs_t*	rss,	// Reference Signal Sequence
							int		ns,		// Slot number
							int		l,		// OFDM symbol
							int		Nid,	// Cell Identification number
							int		Ncp		// Cyclic prefix type (1 for normal CP, 0 for extended CP)
						) {
	pn_t Cn [4*Ndlrbmax];

	unsigned int cinit = (2<<9)*(7*(ns+1)+l+1)*(2*Nid+1)+2*Nid+Ncp;
	int m;

	/* Generate pseudo-random gold sequence */
	goldGen (Cn, cinit, 4*Ndlrbmax);

	/* Generate Reference signals Sequences */
	for (m = 0; m < 2*Ndlrbmax; m++)
	{
		rss[m] = (float)(1-2*(int)Cn[2*m])/SQ2 + I*(float)(1-2*(int)Cn[2*m+1])/SQ2;
	}
}

/*
 * Mapping Reference Signals to Resource Elements.
 * It follows the 3GPP Release 8 (LTE) 36.211
 * Section 6.10.1.2
 */
int mapRS (	int 	Ndlrb,	// Number of PRBs in a downlink ofdm symbol
					int		p,		// Antenna Port (0, 1, 2, 3)
					int		l,		// OFDM symbol in slot
					int 	ns,		// Number of slot
					int		Nid,	// Cell identifier
					rs_t*	rss,	// Reference Symbols Sequences
					rs_t*	rs,		// Reference Symbols Signals
					int*	rspos,	// Reference Symbols positions in frame
					int		Ndlsym,	// Number of ODFDM symbols in a downlink slot
					int		fseg	// Frame segmentation that is received each processing TS
		){
	int m, mp, k;
	int v, vshift;
	int count = 0;
	int guard, fftsize;

	/* Get FFT size */
	if (Ndlrb <= 6) {
		guard = (128-Ndlrb*12)/2;
		fftsize = 128;
	}
	else if (Ndlrb <= 12) {
		guard = (256-Ndlrb*12)/2;
		fftsize = 256;
	}
	else if (Ndlrb <= 25) {
		guard = (512-Ndlrb*12)/2;
		fftsize = 512;
	}
	else if (Ndlrb <= 50) {
		guard = (1024-Ndlrb*12)/2;
		fftsize = 1024;
	}
	else if (Ndlrb <= 75) {
		guard = (1536-Ndlrb*12)/2;
		fftsize = 1536;
	}
	else {
		guard = (2048-Ndlrb*12)/2;
		fftsize = 2048;
	}


	/* Get v variable */
	if (p == 0 && l == 0){
		v = 0;
	}else if (p == 0 && l != 0){
		v = 3;
	}else if (p == 1 && l == 0){
		v = 3;
	}else if (p == 1 && l != 0){
		v = 0;
	}else{
		v = 3 + 3*(ns%2);
	}

	/* Get vshift variable */
	vshift = Nid%6;

	for (m = 0; m<2*Ndlrb; m++)
	{
		k = 6*m + (v+vshift)%6;
		mp = m + Ndlrbmax - Ndlrb;
		rspos[m] = (fftsize*(l+ns*Ndlsym))%(fftsize*140/fseg) + guard + k;	// Stores the position of the RS in the fft.
		rs[m] = rss[mp];
	}

	return m;
}

int setRefSignals (	int		Ndlrb,		// Number of PRBs in a downlink ofdm symbol
					int 	Nid,		// Cell identifier number
					int 	Ncp,		// Cyclic prefix type (1 for normal CP, 0 for extended CP)
					int		Ndlsym,		// Number of ODFDM symbols in a downlink slot
					int		p, 			// Antenna Port (0, 1, 2, 3)
					rs_t*	rs,			// Reference signals pointer
					int*	rspos,		// Reference signals positions
					int		fseg		// Frame segmentation that is received each processing TS
					) {
	int ns, i;
	int count = 0;

	rs_t rss [2*Ndlrbmax];

	// ns -> Slot number in radioframe
	// l -> OFDM number in slot
	for (ns = 0; ns<NUMSLOTS; ns++)
	{
		/* For Antenna ports 0 and 1 */
		if (p==0 || p==1) {
			/* Generate RS sequences for OFDM symbol 0 */
			genRSSequence(rss, ns, 0, Nid, Ncp);
			count += mapRS (Ndlrb, p, 0, ns, Nid, rss, rs+count, rspos+count, Ndlsym, fseg);

			/* Generate RS sequences for OFDM symbol Ndlsym-3 */
			genRSSequence(rss, ns, Ndlsym-3, Nid, Ncp);
			count += mapRS (Ndlrb, p, Ndlsym-3, ns, Nid, rss, rs+count, rspos+count, Ndlsym, fseg);

		/* For Antenna pots 2 and 3 */
		} else if (p==2 || p==3) {
			/* Generate RS sequences for OFDM symbol 1 */
			genRSSequence(rss, ns, 1, Nid, Ncp);
			count += mapRS (Ndlrb, p, 1, ns, Nid, rss, rs+count, rspos+count, Ndlsym, fseg);

		/* Unknown antenna port */
		} else {
			return ERR_BADAP;
		}
	}

	/* If Bad segmentation parameter selection */
	if (count%fseg!=0){
		return ERR_BADSEG;
	}

	return count;
}


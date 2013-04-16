/*
 * syncSignals.h
 *
 *  Created on: Feb 6, 2012
 *      Author: xavier
 */

#ifndef _REFSIGNALS_H_
#define _REFSIGNALS_H_

/* Working type for reference symbols */
typedef _Complex float rs_t;

/* Working type for pseudo random sequences */
typedef char pn_t;

/* Maximums definitions */
#define MAXRS	(4*1200*140/7/12)

/* ERRORS DEFINITION */
#define ERR_BADAP	-1	// Bad antenna port parameter
#define	ERR_BADSEG	-2	// Bad segmentation parameter

/* Reference signals generation function declaration */
int setRefSignals (	int		Ndlrb,		// Number of PRBs in a downlink ofdm symbol
					int 	Nid,		// Cell identifier number
					int 	Ncp,		// Cyclic prefix type (1 for normal CP, 0 for extended CP)
					int		Ndlsym,		// Number of ODFDM symbols in a downlink slot
					int		p, 			// Antenna Port (0, 1, 2, 3)
					rs_t*	rs,			// Reference signals pointer
					int*	rspos,		// Reference signals positions
					int		fseg		// Frame segmentation that is received each processing TS
			);

#endif /* REFSIGNALS_H_ */

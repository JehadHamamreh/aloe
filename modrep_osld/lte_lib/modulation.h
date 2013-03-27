
#define BPSK    1
#define QPSK    2
#define QAM16   4
#define QAM64   6


/* The symbols are specified in [3GPP TS 36.211 version 10.5.0 Release 10, Section 7] */
/* from which the symbols levels follow: */
#define BPSK_LEVEL      1/sqrt(2)

#define QPSK_LEVEL      1/sqrt(2)

#define QAM16_LEVEL_1	1/sqrt(10)
#define QAM16_LEVEL_2	3/sqrt(10)

#define QAM64_LEVEL_1	1/sqrt(42)
#define QAM64_LEVEL_2	3/sqrt(42)
#define QAM64_LEVEL_3	5/sqrt(42)
#define QAM64_LEVEL_4	7/sqrt(42)

#define QAM64_LEVEL_x	2/sqrt(42)		/* this is not an QAM64 level, but, rather, an auxiliary value that can be used for computing the symbol from the bit sequence */

/* Thresholds for Demodulation */
/* Assume perfect amplitude and phase alignment.
 *  Check threshold values for real case
 *  or implement dynamic threshold adjustent as a function of received symbol amplitudes */
#define QAM16_THRESHOLD		2/sqrt(10)
#define QAM64_THRESHOLD_1	2/sqrt(42)
#define QAM64_THRESHOLD_2	4/sqrt(42)
#define QAM64_THRESHOLD_3	6/sqrt(42)

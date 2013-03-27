
/* LTE specifies QPSK, 16QAM and 64QAM for data and BPSK for some control channels */
#define BPSK    1
#define QPSK    2
#define QAM16   3
#define QAM64   4

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

/* HARD DEMODULATION Thresholds */
#define QAM16_THRESHOLD		2/sqrt(10)

#define QAM64_THRESHOLD_1	2/sqrt(42)
#define QAM64_THRESHOLD_2	4/sqrt(42)
#define QAM64_THRESHOLD_3	6/sqrt(42)

/* Function prototypes */
int get_bits_per_symbol(int modulation);
void hard_demod(input_t *in, output_t *out, int N, int modulation);
void hard_demod_real(float *in, output_t *out, int N, int modulation);

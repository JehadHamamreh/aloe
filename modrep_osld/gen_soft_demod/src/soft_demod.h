
/* LTE specifies QPSK, 16QAM and 64QAM for data and BPSK for some control channels */
#define BPSK    0
#define QPSK    1
#define QAM16   2
#define QAM64   3

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

struct constellation_tables {
	_Complex float bpsk[2];
	_Complex float qpsk[4];
	_Complex float qam16[16];
	_Complex float qam64[64];
};

/* symbol indices with a 0 and 1: first dim: S0|S1, 2nd dim: bit position, 3rd dim: symbol index */
struct Sx {
	int bpsk[2][6][32];	/* int bpsk[2][1][1];*/
	int qpsk[2][6][32];	/* int qpsk[2][2][2];*/
	int qam16[2][6][32];	/* int qam16[2][4][8];*/
	int qam64[2][6][32];
};

/* Function prototypes */
//void set_modulation_tables(struct constellation_tables *table, struct Sx *Sx);
void set_BPSKtable(_Complex float *bpsk_table, int (*S)[6][32]);
void set_QPSKtable(_Complex float *qpsk_table, int (*S)[6][32]);
void set_16QAMtable(_Complex float *qam16_table, int (*S)[6][32]);
void set_64QAMtable(_Complex float *qam64_table, int (*S)[6][32]);
int get_bits_per_symbol(int modulation);
void llr_approx(input_t *in, output_t *out, int N, int M, int B,
	_Complex float *symbols, int (*S)[6][32], float sigma2);
void llr_exact(input_t *in, output_t *out, int N, int M, int B,
	_Complex float *symbols, int (*S)[6][32], float sigma2);

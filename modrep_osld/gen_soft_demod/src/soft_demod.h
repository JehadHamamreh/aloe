
#include "lte_lib/modulation.h"

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
void llr_approx(_Complex float *in, output_t *out, int N, int M, int B,
	_Complex float *symbols, int (*S)[6][32], float sigma2);
void llr_exact(_Complex float *in, output_t *out, int N, int M, int B,
	_Complex float *symbols, int (*S)[6][32], float sigma2);

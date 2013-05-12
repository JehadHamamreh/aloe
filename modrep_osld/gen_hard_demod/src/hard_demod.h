
#include "lte_lib/modulation.h"

/* Function prototypes */
int get_bits_per_symbol(int modulation);
void hard_demod(input_t *in, output_t *out, int N, int modulation);
void hard_demod_real(float *in, output_t *out, int N, int modulation);

void hard_bpsk_demod(input_t *in, output_t *out, int N);
void hard_bpsk_demod_real(float *in, output_t *out, int N);
void hard_qpsk_demod(input_t *in, output_t *out, int N);
void hard_qam16_demod(input_t *in, output_t *out, int N);
void hard_qam64_demod(input_t *in, output_t *out, int N);


#define INCLUDE_DEFS_ONLY
#include "gen_modulator.h"


#include "lte_lib/modulation.h"




/* Moduation Mapper/Demapper functions
 * INIT phase */
void set_BPSKtable(void);
void set_QPSKtable(void);
void set_16QAMtable(void);
void set_64QAMtable(void);

int get_bits_per_symbol(int modulation);

void modulate_BPSK_real(input_t *bits, float *S_out);
void modulate_BPSK(input_t *bits, output_t *S_out);
void modulate_QPSK(input_t *bits, output_t *S_out);
void modulate_16QAM(input_t *bits, output_t *S_out);
void modulate_64QAM(input_t *bits, output_t *S_out);

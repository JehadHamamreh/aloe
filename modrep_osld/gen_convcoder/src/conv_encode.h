
typedef int uint32;
typedef char uint8;

void conv_encode(uint8             *c_bits,
                 uint32             N_c_bits,
                 uint32             constraint_len,
                 uint32             rate,
                 uint32            *g,
                 int               tail_bit,
                 uint8             *d_bits,
                 uint32            *N_d_bits);

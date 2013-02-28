
/* CFI is encoded with 32 bits */
#define NOF_BITS	32


/* Function prototypes */
void coding_table(char (*table)[NOF_BITS]);
int cfi_decoding(char *in, char (*table)[NOF_BITS]);

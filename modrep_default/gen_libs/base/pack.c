
void pack_bits(unsigned int value, char **bits, int nof_bits)
{
    int i;

    for(i=0; i<nof_bits; i++) {
        (*bits)[i] = (value >> (nof_bits-i-1)) & 0x1;
    }
    *bits += nof_bits;
}

unsigned int unpack_bits(char **bits, int nof_bits)
{
    int i;
    unsigned int value=0;

    for(i=0; i<nof_bits; i++) {
    	value |= (*bits)[i] << (nof_bits-i-1);
    }
    *bits += nof_bits;
    return value;
}

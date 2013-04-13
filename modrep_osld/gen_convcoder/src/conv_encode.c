
#include "conv_encode.h"

/**
 * This function is taken from:
 * Author: Ben Wojtowicz (http://openlte.sourceforge.net/)
 * License: GNU Affero General Public License
 */

/*********************************************************************
    Name: conv_encode

    Description: Convolutionally encodes a bit array using the
                 provided parameters

    Document Reference: 3GPP TS 36.212 v10.1.0 section 5.1.3.1
*********************************************************************/
void conv_encode(uint8             *c_bits,
                 uint32             N_c_bits,
                 uint32             constraint_len,
                 uint32             rate,
                 uint32            *g,
                 int               tail_bit,
                 uint8             *d_bits,
                 uint32            *N_d_bits)
{
    uint32 i;
    uint32 j;
    uint32 k;
    uint8  s_reg[constraint_len];
    uint8  g_array[3][constraint_len];

    /* Initialize the shift register */
    if(tail_bit)
    {
        for(i=0; i<constraint_len; i++)
        {
            s_reg[i] = c_bits[N_c_bits-i-1];
        }
    }else{
        for(i=0; i<constraint_len; i++)
        {
            s_reg[i] = 0;
        }
    }

    /* Convert g from octal to binary array */
    for(i=0; i<rate; i++)
    {
        for(j=0; j<constraint_len; j++)
        {
            g_array[i][j] = (g[i] >> (constraint_len-j-1)) & 1;
        }
    }

    /* Convolutionally encode input */
    for(i=0; i<N_c_bits; i++)
    {
        /* Add next bit to shift register */
        for(j=constraint_len-1; j>0; j--)
        {
            s_reg[j] = s_reg[j-1];
        }
        s_reg[0] = c_bits[i];

        /* Determine the output bits */
        for(j=0; j<rate; j++)
        {
            d_bits[i*rate + j] = 0;

            for(k=0; k<constraint_len; k++)
            {
                d_bits[i*rate + j] += s_reg[k]*g_array[j][k];
            }
            d_bits[i*rate + j] %= 2;
        }
    }

    *N_d_bits = N_c_bits*rate;
}

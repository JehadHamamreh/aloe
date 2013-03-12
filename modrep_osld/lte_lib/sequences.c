#include <stdlib.h>

#include "sequences.h"

/*********************************************************************
	Author: Ben Wojtowicz (http://openlte.sourceforge.net/)
	License: GNU Affero General Public License

    Name: generate_prs_c

    Description: Generates the psuedo random sequence c

    Document Reference: 3GPP TS 36.211 v10.1.0 section 7.2
*********************************************************************/
void generate_prs_c(uint32  c_init,
                    uint32  len,
                    uint32 *c)
{
    uint32 i;
    uint32 j;
    uint8  x1[31];
    uint8  x2[31];
    uint8  new_bit1;
    uint8  new_bit2;

    // Initialize the m-sequences
    for(i=0; i<31; i++)
    {
        x1[i] = 0;
        x2[i] = (c_init & (1<<i))>>i;
    }
    x1[0] = 1;

    // Advance m-sequences
    for(i=0; i<(1600-31); i++)
    {
        new_bit1 = x1[3] ^ x1[0];
        new_bit2 = x2[3] ^ x2[2] ^ x2[1] ^ x2[0];

        for(j=0; j<30; j++)
        {
            x1[j] = x1[j+1];
            x2[j] = x2[j+1];
        }
        x1[30] = new_bit1;
        x2[30] = new_bit2;
    }

    // Generate c
    for(i=0; i<len; i++)
    {
        new_bit1 = x1[3] ^ x1[0];
        new_bit2 = x2[3] ^ x2[2] ^ x2[1] ^ x2[0];

        for(j=0; j<30; j++)
        {
            x1[j] = x1[j+1];
            x2[j] = x2[j+1];
        }
        x1[30] = new_bit1;
        x2[30] = new_bit2;

        c[i] = new_bit1 ^ new_bit2;
    }
}

#include <math.h>
#include "viterbi_decoder.h"

/**
 * This function is taken from:
 * Author: Ben Wojtowicz (http://openlte.sourceforge.net/)
 * License: GNU Affero General Public License
 */

/*********************************************************************
    Name: viterbi_decode

    Description: Viterbi decodes a convolutionally coded input bit
                 array using the provided parameters

    Document Reference: 3GPP TS 36.212 v10.1.0 section 5.1.3.1
*********************************************************************/
void viterbi_decode(struct viterbi *viterbi,
                    float             *d_bits,
                    uint32             N_d_bits,
                    uint32             constraint_len,
                    uint32             rate,
                    uint32            *g,
                    uint8             *c_bits,
                    uint32            *N_c_bits)
{
    float  init_min;
    float  tmp1;
    float  tmp2;
    int32  i;
    uint32 j;
    uint32 k;
    uint32 o;
    uint32 N_states = 1<<(constraint_len-1);
    uint32 idx;
    uint32 tb_state_len;
    uint8  in_path;
    uint8  prev_state;
    uint8  in_bit;
    uint8  prev_state_0;
    uint8  prev_state_1;
    uint8  s_reg[constraint_len];
    uint8  g_array[3][constraint_len];

    /* Convert g to binary */
    for(i=0; i<(int32)rate; i++)
    {
        for(j=0; j<constraint_len; j++)
        {
            g_array[i][j] = (g[i] >> (constraint_len-j-1)) & 1;
        }
    }

    /* Precalculate state transition outputs */
    for(i=0; i<(int32)N_states; i++)
    {
        /* Determine the input path */
        if(i < (N_states/2))
        {
            in_path = 0;
        }else{
            in_path = 1;
        }

        /* Determine the outputs based on the previous state and input path */
        for(j=0; j<2; j++)
        {
            prev_state = ((i << 1) + j) % N_states;
            for(k=0; k<constraint_len; k++)
            {
                s_reg[k] = (prev_state >> (constraint_len-k-1)) & 1;
            }
            s_reg[0] = in_path;
            for(k=0; k<rate; k++)
            {
                viterbi->vd_st_output[i][j][k] = 0;
                for(o=0; o<constraint_len; o++)
                {
                    viterbi->vd_st_output[i][j][k] += s_reg[o]*g_array[k][o];
                }
                viterbi->vd_st_output[i][j][k] %= 2;
            }
        }
    }

    /* Calculate branch and path metrics */
    for(i=0; i<(int32)N_states; i++)
    {
        for(j=0; j<(N_d_bits/rate)+10; j++)
        {
            viterbi->vd_path_metric[i][j] = 0;
        }
    }
    for(i=0; i<(int32)(N_d_bits/rate); i++)
    {
        for(j=0; j<N_states; j++)
        {
            viterbi->vd_br_metric[j][0] = 0;
            viterbi->vd_br_metric[j][1] = 0;
            viterbi->vd_p_metric[j][0]  = 0;
            viterbi->vd_p_metric[j][1]  = 0;
            viterbi->vd_w_metric[j][0]  = 0;
            viterbi->vd_w_metric[j][1]  = 0;

            /* Calculate the accumulated branch metrics for each state */
            for(k=0; k<2; k++)
            {
                prev_state                    = ((j<<1)+k) % N_states;
                viterbi->vd_p_metric[j][k] = viterbi->vd_path_metric[(int)prev_state][i];
                for(o=0; o<rate; o++)
                {
                    if(d_bits[i*rate + o] <= 0)
                    {
                        in_bit = 0;
                    }else{
                        in_bit = 1;
                    }
                    viterbi->vd_br_metric[j][k] += (viterbi->vd_st_output[j][k][o]+in_bit)%2;
                    viterbi->vd_w_metric[j][k]  += fabs(d_bits[i*rate + o]);
                }
            }

            /* Keep the smallest branch metric as the path metric, weight the branch metric */
            tmp1 = viterbi->vd_br_metric[j][0] + viterbi->vd_p_metric[j][0];
            tmp2 = viterbi->vd_br_metric[j][1] + viterbi->vd_p_metric[j][1];
            if(tmp1 > tmp2)
            {
                viterbi->vd_path_metric[j][i+1] = viterbi->vd_p_metric[j][1] + viterbi->vd_w_metric[j][1]*viterbi->vd_br_metric[j][1];
            }else{
                viterbi->vd_path_metric[j][i+1] = viterbi->vd_p_metric[j][0] + viterbi->vd_w_metric[j][0]*viterbi->vd_br_metric[j][0];
            }
        }
    }

    /* Find the minimum metric for the last iteration */
    init_min                     = 1000000;
    idx                          = 0;
    viterbi->vd_tb_state[idx] = 1000000;
    for(i=0; i<(int32)N_states; i++)
    {
        if(viterbi->vd_path_metric[i][(N_d_bits/rate)] < init_min)
        {
            init_min                       = viterbi->vd_path_metric[i][(N_d_bits/rate)];
            viterbi->vd_tb_state[idx++] = i;
        }
    }

    /* Traceback to find the minimum path metrics at each iteration */
    for(i=(N_d_bits/rate)-1; i>=0; i--)
    {
        prev_state_0 = ((((uint8)viterbi->vd_tb_state[idx-1])<<1) + 0) % N_states;
        prev_state_1 = ((((uint8)viterbi->vd_tb_state[idx-1])<<1) + 1) % N_states;

        /* Keep the smallest state */
        if(viterbi->vd_path_metric[(int)prev_state_0][i] > viterbi->vd_path_metric[(int)prev_state_1][i])
        {
            viterbi->vd_tb_state[idx++] = prev_state_1;
        }else{
            viterbi->vd_tb_state[idx++] = prev_state_0;
        }
    }
    tb_state_len = idx;

    /* Read through the traceback to determine the input bits */
    idx = 0;
    for(i=tb_state_len-2; i>=0; i--)
    {
        /* If transition has resulted in a lower valued state, */
        /* the output is 0 and vice-versa */
        if(viterbi->vd_tb_state[i] < viterbi->vd_tb_state[i+1])
        {
            c_bits[idx++] = 0;
        }else if(viterbi->vd_tb_state[i] > viterbi->vd_tb_state[i+1]){
            c_bits[idx++] = 1;
        }else{
            /* Check to see if the transition has resulted in the same state */
            /* In this case, if state is 0 then output is 0 */
            if(viterbi->vd_tb_state[i] == 0)
            {
                c_bits[idx++] = 0;
            }else{
                c_bits[idx++] = 1;
            }
        }
    }
    *N_c_bits = idx;
}

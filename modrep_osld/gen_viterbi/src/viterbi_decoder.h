
typedef int uint32;
typedef char uint8;
typedef int int32;

/* Viterbi decoder structure, adapted from liblte_phy.h */
struct viterbi {
    float vd_path_metric[128][2048];
    float vd_br_metric[128][2];
    float vd_p_metric[128][2];
    float vd_w_metric[128][2048];
    float vd_tb_state[2048];
    uint8 vd_st_output[128][2][3];
};

void viterbi_decode(struct viterbi *viterbi,
                    float             *d_bits,
                    uint32             N_d_bits,
                    uint32             constraint_len,
                    uint32             rate,
                    uint32            *g,
                    uint8             *c_bits,
                    uint32            *N_c_bits);

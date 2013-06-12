
#define SSS_DFT_LEN 128

void convert_tables(struct sss_tables *in);
int get_m0m1(complex_t *input, int *m0, int *m1, int correlation_threshold);
int decide_subframe(int m0, int m1);
int decide_N_id_1(int m0, int m1);

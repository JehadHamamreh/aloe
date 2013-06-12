
struct conv_fft_cc {
	complex_t *input_fft;
	complex_t *filter_fft;
	complex_t *output_fft;
	complex_t *output_fft2;
	int input_len;
	int filter_len;
	int output_len;
	dft_plan_t input_plan;
	dft_plan_t filter_plan;
	dft_plan_t output_plan;

};
int conv_fft_cc_init(struct conv_fft_cc *state, int input_len, int filter_len);
int conv_fft_cc_run(struct conv_fft_cc *state, complex_t *input, complex_t *filter, complex_t *output);

int conv_cc(complex_t *input, complex_t *filter, complex_t *output, int input_len, int filter_len);

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "dft/dft.h"
#include "vector.h"
#include "conv.h"


int conv_fft_cc_init(struct conv_fft_cc *state, int input_len, int filter_len) {
	state->input_len = input_len;
	state->filter_len = filter_len;
	state->output_len = input_len+filter_len-1;
	state->input_fft = vec_malloc(sizeof(complex_t)*state->output_len);
	state->filter_fft = vec_malloc(sizeof(complex_t)*state->output_len);
	state->output_fft = vec_malloc(sizeof(complex_t)*state->output_len);
	if (!state->input_fft || !state->filter_fft || !state->output_fft) {
		return -1;
	}
	if (dft_plan(state->output_len,COMPLEX_2_COMPLEX,FORWARD,&state->input_plan)) {
		return -2;
	}
	if (dft_plan(state->output_len,COMPLEX_2_COMPLEX,FORWARD,&state->filter_plan)) {
		return -3;
	}
	if (dft_plan(state->output_len,COMPLEX_2_COMPLEX,BACKWARD,&state->output_plan)) {
		return -4;
	}
	return 0;
}

int conv_fft_cc_run(struct conv_fft_cc *state, complex_t *input, complex_t *filter, complex_t *output) {

	dft_run_c2c(&state->input_plan, input, state->input_fft);
	dft_run_c2c(&state->filter_plan, filter, state->filter_fft);

	vec_dot_prod(state->input_fft,state->filter_fft,state->output_fft,state->output_len);
	//vec_mult_c_r(state->output_fft,state->output_fft2,(float) 1/state->output_len,state->output_len);

	dft_run_c2c(&state->output_plan, state->output_fft, output);

	return state->output_len;

}

int conv_cc(complex_t *input, complex_t *filter, complex_t *output, int input_len, int filter_len) {
	int i,j;
	int output_len;
	output_len=input_len+filter_len-1;
	memset(output,0,output_len*sizeof(complex_t));
	for (i=0;i<input_len;i++) {
		for (j=0;j<filter_len;j++) {
			output[i+j]+=input[i]*filter[j];
		}
	}
	return output_len;
}

/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <oesr.h>
#include <params.h>
#include <math.h>
#include <complex.h>
#include <skeleton.h>
#include <string.h>

#include "dft/dft.h"
#include "base/types.h"
#include "base/conv.h"
#include "base/vector.h"
#include "utils/nco.h"
#include "lte_pss_synch.h"
#include "lte_lib/grid/base.h"

#define PSS_LEN_FREQ 129
#define NOT_SYNC	0xF0F0F0F0

static int conv_fft=0,input_len,bypass,correlation_threshold,do_cfo;
static pmid_t manual_cfo;
static int frame_start_idx,fb_wp,unsync_nof_pkts;

struct conv_fft_cc fft_state;

complex_t pss_signal_time[PSS_LEN];
complex_t pss_signal_pad[PSS_LEN_FREQ];
complex_t pss_signal_freq[INPUT_MAX_SAMPLES];

static real_t cfo=0.0;
static int nof_nosync_pkts=0;

real_t conv_abs[INPUT_MAX_SAMPLES];
complex_t frame_buffer[INPUT_MAX_SAMPLES];
complex_t conv_output[INPUT_MAX_SAMPLES];
complex_t tmp_nco[INPUT_MAX_SAMPLES];

void generate_pss_filter(int N_id_2, int input_len);
int find_pss(input_t *input, input_t *filter, real_t *cfo, int input_len);
void correct_cfo(input_t *signal, real_t cfo, int len);

nco_t nco;

int initialize() {
	int N_id_2;

	N_id_2=0;
	param_get_int_name("N_id_2",&N_id_2);

	conv_fft=1;
	param_get_int_name("conv_fft",&conv_fft);

	input_len=0;
	param_get_int_name("input_len",&input_len);

	unsync_nof_pkts=0;
	param_get_int_name("unsync_nof_pkts",&unsync_nof_pkts);

	do_cfo=1;
	param_get_int_name("do_cfo",&do_cfo);

	manual_cfo=param_id("manual_cfo");

	bypass=0;
	param_get_int_name("bypass",&bypass);

	correlation_threshold=10000;
	param_get_int_name("correlation_threshold",&correlation_threshold);

	if (conv_fft && !input_len) {
		moderror("Parameter input_len must be greater than 0\n");
		return -1;
	}

	if (conv_fft) {
		if (conv_fft_cc_init(&fft_state, input_len, PSS_LEN_FREQ)) {
			moderror("Creating fft plan\n");
			return -1;
		}
	}

	generate_pss_filter(N_id_2,input_len);

	frame_start_idx=NOT_SYNC;
	fb_wp=0;

	nco_init(&nco,1024*1024);

	return 0;
}

int work(void **inp, void **out) {
	int input_len,max_idx,tmp_start_idx;
	int output_len;
	input_t *input;
	output_t *output;

	input_len=get_input_samples(0);
	input=inp[0];
	output=out[0];
	output_len=0;
	if (!input_len) {
		return 0;
	}

	if (bypass) {
		memcpy(output,input,input_len*sizeof(input_t));
		output_len=input_len;
		goto fix_cfo;
	}
	modinfo_msg("rcv %d samples\n",input_len);
	max_idx = find_pss(input,pss_signal_freq,&cfo,input_len);
	if (max_idx>=0) {
		tmp_start_idx = max_idx-input_len/2;
		if (frame_start_idx != tmp_start_idx) {
			modinfo_msg("Re-synchronizing: new index is %d, old was %d\n",tmp_start_idx,frame_start_idx);
		}
		frame_start_idx = tmp_start_idx;
	} else {
		if (unsync_nof_pkts>0) {		
			nof_nosync_pkts++;
			if (nof_nosync_pkts>=unsync_nof_pkts*10) {
				frame_start_idx = NOT_SYNC;
			}
		}
	}

	if (frame_start_idx == NOT_SYNC) {

		memcpy(frame_buffer,input,input_len*sizeof(input_t));
		output_len = 0;

	} else if (frame_start_idx > 0) {

		if (fb_wp) {
			memcpy(&frame_buffer[(input_len-frame_start_idx)],input,frame_start_idx*sizeof(input_t));
			memcpy(output,frame_buffer,input_len*sizeof(input_t));
			output_len = input_len;
		}
		memcpy(frame_buffer,&input[frame_start_idx],(input_len-frame_start_idx)*sizeof(input_t));
		fb_wp=1;

	} else {

		memcpy(output,&frame_buffer[input_len+frame_start_idx],(-frame_start_idx)*sizeof(input_t));
		memcpy(&output[-frame_start_idx],input,(input_len+frame_start_idx)*sizeof(input_t));
		memcpy(&frame_buffer[input_len+frame_start_idx],&input[input_len+frame_start_idx],
				(-frame_start_idx)*sizeof(input_t));
		output_len = input_len;
	}

fix_cfo:
	if (do_cfo) {
		if (manual_cfo) {
			param_get_float(manual_cfo,&cfo);
		}
		correct_cfo(output,cfo,output_len);
	}

	return output_len;

}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

complex_t y[PSS_LEN_FREQ-1];

int find_pss(input_t *input, input_t *filter, real_t *cfo, int input_len) {
	int max_pos;
	int len;
	real_t max;
	complex_t y0,y1,yr;
	real_t tan;

	memset(&filter[PSS_LEN_FREQ],0,input_len*sizeof(complex_t));
	memset(&input[input_len],0,PSS_LEN_FREQ*sizeof(complex_t));

	if (conv_fft) {
		len = conv_fft_cc_run(&fft_state, input, filter, conv_output);
	} else {
		len = conv_cc(input, filter, conv_output, input_len, PSS_LEN_FREQ);
	}

	vec_abs(conv_output, conv_abs, len);
	vec_max(conv_abs, &max, &max_pos, len);

	if (max<correlation_threshold) {
		return -1;
	}

	if (do_cfo && max_pos>PSS_LEN_FREQ-1) {
		vec_dot_prod_u(filter,&input[max_pos-PSS_LEN_FREQ+1],y,PSS_LEN_FREQ-1);

		y0=sum_c(y,64);
		y1=sum_c(&y[64],64);
		yr=conjf(y0)*y1;
		tan=(__imag__ yr)/(__real__ yr);
		*cfo=atanf(tan)/M_PI;
	}

	modinfo_msg("Peak detected %g at %d, cfo=%g\n",max,max_pos,*cfo);

	return max_pos;
}

void generate_pss_filter(int N_id_2, int input_len) {
	struct lte_grid_config config;
	dft_plan_t plan;

	config.cell_id = N_id_2;
	config.debug=0;
	generate_pss(pss_signal_time, 0, &config);

	memset(pss_signal_pad,0,PSS_LEN_FREQ*sizeof(complex_t));
	memset(pss_signal_freq,0,PSS_LEN_FREQ*sizeof(complex_t));
	memcpy(&pss_signal_pad[33],pss_signal_time,PSS_LEN*sizeof(complex_t));

	dft_plan(PSS_LEN_FREQ-1,COMPLEX_2_COMPLEX,BACKWARD,&plan);
	plan.options = DFT_MIRROR_PRE | DFT_DC_OFFSET;

	dft_run_c2c(&plan, pss_signal_pad, pss_signal_freq);

	vec_mult_c_r(pss_signal_freq, pss_signal_pad, (float) 1/(PSS_LEN_FREQ-1), PSS_LEN_FREQ);

	vec_conj(pss_signal_pad, pss_signal_freq,PSS_LEN_FREQ);

}

void correct_cfo(input_t *signal, real_t cfo, int len) {
	modinfo_msg("correcting cfo=%g\n",cfo);
	nco_cexp_f(&nco,conv_output,-len*cfo/128,len);
	vec_dot_prod(signal,conv_output,signal,len);
}

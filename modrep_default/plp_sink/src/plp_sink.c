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

#include <stdlib.h>

#include <oesr.h>
#include <skeleton.h>
#include <params.h>
#include <str.h>

#include "plp_sink.h"
#include "plp.h"
#include "dft/dft.h"

pmid_t mode_id, fftsize_id;

void print_signal(void *input, int len);

static char r_legends[2*NOF_INPUT_ITF][STR_LEN];
static char c_legends[2*NOF_INPUT_ITF][STR_LEN];
static char fft_legends[2*NOF_INPUT_ITF][STR_LEN];
static int signal_lengths[2*NOF_INPUT_ITF];

static float f_pl_signals[2*NOF_INPUT_ITF*INPUT_MAX_SAMPLES];
static double pl_signals[2*NOF_INPUT_ITF*INPUT_MAX_SAMPLES];
static int plp_initiated=0;
static int fft_initiated=0;
static int interval_ts, last_tstamp;
static int print_not_received;
static int last_rcv_samples;
static int is_complex;
int fft_size;

const int precomputed_dft_len[] = {1024,960,1920,3840,7680};
#define NOF_PRECOMPUTED_DFT 5
dft_plan_t plans[NOF_PRECOMPUTED_DFT];
#define MAX_EXTRA_PLANS	5
dft_plan_t extra_plans[MAX_EXTRA_PLANS];

void setup_legends();


dft_plan_t* find_plan(int dft_size) {
	int i;
	for (i=0;i<NOF_PRECOMPUTED_DFT;i++) {
		if (plans[i].size == dft_size) {
			return &plans[i];
		}
	}
	return NULL;
}


dft_plan_t* generate_new_plan(int dft_size) {
	int i;

	modinfo_msg("Warning, no plan was precomputed for size %d. Generating.\n",dft_size);
	for (i=0;i<MAX_EXTRA_PLANS;i++) {
		if (!extra_plans[i].size) {
			if (is_complex) {
				if (dft_plan_c2r(dft_size, FORWARD, &extra_plans[i])) {
					return NULL;
				}
			} else {
				if (dft_plan_r2r(dft_size, FORWARD, &extra_plans[i])) {
					return NULL;
				}
			}
			extra_plans[i].options = DFT_PSD | DFT_OUT_DB | DFT_NORMALIZE;
			return &extra_plans[i];
		}
	}
	return NULL;
	}

/**
 * @ingroup plp_sink
 *
 *  Initializes the plplot driver if mode is SCOPE or PSD. If the mode is PSD, it
 * also initializes computes the fftw3 plan for the selected dft_size.
 *
 * \param is_complex 0: The input data for all interfaces is real;
 * 1: The input data for all interfaces is complex. This parameter is mandatory.
 * \param mode 0: Do nothing; 1: Print to stdout the received samples; 2: SCOPE mode, plots the
 * received signal using plplot; 3: PSD mode, plots the power spectral density of the received signal.
 * Default is 0 (silent)
 * \param fft_size Size of the DFT to compute the PSD. Default is 128.
 */
int initialize() {
	int i;
	int mode;
	int tslen;
	int data_type;

	last_rcv_samples=0;

	memset(plans,0,sizeof(dft_plan_t)*NOF_PRECOMPUTED_DFT);
	memset(extra_plans,0,sizeof(dft_plan_t)*MAX_EXTRA_PLANS);

	setup_legends();

	if (param_get_int_name("data_type", &data_type)) {
		data_type = 0;
	}
	switch(data_type) {
	case 0:
		is_complex = 0;
		break;
	case 1:
		is_complex = 1;
		break;
	case 2:
		moderror("Only data_type 0 or 1 is supported\n");
		return -1;
	}

	if (param_get_int_name("print_not_received",&print_not_received)!=1) {
		print_not_received = 0;
	}

	mode_id = param_id("mode");
	if (mode_id == NULL) {
		modinfo("Mode is not configured, using default mode 'silent'\n");
	} else {
		if (param_get_int(mode_id,&mode) != 1) {
			moderror("Error getting parameter mode\n");
			return -1;
		}
		if (mode == MODE_SCOPE || mode == MODE_PSD) {
			modinfo("Initiating plplot...\n");
			if (plp_init(PL_DRIVER,"",is_complex)) {
				return -1;
			}
			plp_initiated = 1;
			reset_axis();
			modinfo("-- Warning --: plplot crashes at stop. Restart ALOE after stopping the waveform.\n");
		}
	}

	if (mode == MODE_PSD) {
		fft_size=0;
		param_get_int_name("fft_size",&fft_size);
		if (is_complex) {
			if (dft_plan_multi_c2r(precomputed_dft_len, FORWARD, NOF_PRECOMPUTED_DFT, plans)) {
				moderror("Precomputing plans\n");
				return -1;
			}
		} else {
			if (dft_plan_multi_r2r(precomputed_dft_len, FORWARD, NOF_PRECOMPUTED_DFT, plans)) {
				moderror("Precomputing plans\n");
				return -1;
			}
		}
		for (i=0;i<NOF_PRECOMPUTED_DFT;i++) {
			plans[i].options = DFT_PSD | DFT_OUT_DB | DFT_NORMALIZE;
		}
		fft_initiated = 1;
	}


#ifdef _COMPILE_ALOE
	tslen = oesr_tslot_length(ctx);
	if (tslen > EXEC_MIN_INTERVAL_MS*1000) {
		interval_ts = 1;
	} else {
		interval_ts = (EXEC_MIN_INTERVAL_MS*1000)/tslen;
		moddebug("Timeslot is %d usec, refresh interval set to %d tslots\n",tslen,interval_ts);
	}
	last_tstamp = 0;
#endif

	return 0;
}

/**@ingroup plp_sink
 * Prints or displays the signal according to the selected mode.
 */
int work(void **inp, void **out) {
	int n,i,j;
	int mode;
	float *r_input;
	_Complex float *c_input;
	dft_plan_t *plan;

	strdef(xlabel);

	if (mode_id != NULL) {
		if (param_get_int(mode_id,&mode) != 1) {
			mode = 0;
		}
	} else {
		mode = 0;
	}
	memset(signal_lengths,0,sizeof(int)*2*NOF_INPUT_ITF);
	for (n=0;n<NOF_INPUT_ITF;n++) {
		if (is_complex && mode != MODE_PSD) {
			signal_lengths[2*n] = get_input_samples(n)/2;
			signal_lengths[2*n+1] = signal_lengths[2*n];
		} else {
			signal_lengths[n] = get_input_samples(n);
		}
		if (get_input_samples(n) != last_rcv_samples) {
			last_rcv_samples = get_input_samples(n);
#ifdef _COMPILE_ALOE
			moddebug("Receiving %d samples at tslot %d\n",last_rcv_samples,
					oesr_tstamp(ctx));
#endif
		}
	}

#ifdef _COMPILE_ALOE
	if (print_not_received) {
		for (n=0;n<NOF_INPUT_ITF;n++) {
			if (MOD_DEBUG) {
				ainfo_msg("ts=%d, rcv_len=%d\n",oesr_tstamp(ctx),get_input_samples(n));
			}
			if (!get_input_samples(n)) {
				printf("ts=%d. Data not received from interface %d\n",oesr_tstamp(ctx),n);
			}

		}
	}
#endif


#ifdef _COMPILE_ALOE
	if (oesr_tstamp(ctx)-last_tstamp < interval_ts) {
		return 0;
	}
	last_tstamp = interval_ts;
#endif


	switch(mode) {
	case MODE_SILENT:
		break;
	case MODE_PRINT:
		for (n=0;n<NOF_INPUT_ITF;n++) {
			if (inp[n]) {
				print_signal(inp[n],get_input_samples(n));
			}
		}
	break;
	case MODE_SCOPE:
#ifdef _COMPILE_ALOE
		snprintf(xlabel,STR_LEN,"# sample (ts=%d)",oesr_tstamp(ctx));
#else
		snprintf(xlabel,STR_LEN,"# sample");
#endif
		if (is_complex) {
			set_legend(c_legends,2*NOF_INPUT_ITF);
		} else {
			set_legend(r_legends,NOF_INPUT_ITF);
		}
		set_labels(xlabel,"amp");

		for (n=0;n<NOF_INPUT_ITF;n++) {
			if (inp[n]) {
				if (is_complex) {
					c_input = inp[n];
					for (i=0;i<signal_lengths[2*n];i++) {
						pl_signals[2*n*INPUT_MAX_SAMPLES+i] = (double) __real__ c_input[i];
						pl_signals[(2*n+1)*INPUT_MAX_SAMPLES+i] = (double) __imag__ c_input[i];
					}
				} else {
					r_input = inp[n];
					for (i=0;i<signal_lengths[n];i++) {
						pl_signals[n*INPUT_MAX_SAMPLES+i] = (double) r_input[i];
					}
				}
			}

		}

		plp_draw(pl_signals,signal_lengths,0);
	break;
	case MODE_PSD:
#ifdef _COMPILE_ALOE
		snprintf(xlabel,STR_LEN,"freq. idx (ts=%d)",oesr_tstamp(ctx));
#else
		snprintf(xlabel,STR_LEN,"freq. idx");
#endif

		set_labels(xlabel,"PSD (dB/Hz)");

		set_legend(fft_legends,NOF_INPUT_ITF);

		for (i=0;i<NOF_INPUT_ITF;i++) {
			if (signal_lengths[i]) {
				if (fft_size) {
					signal_lengths[i] = signal_lengths[i]>fft_size?fft_size:signal_lengths[i];
				}
				plan = find_plan(signal_lengths[i]);
				c_input = inp[i];
				r_input = inp[i];
				if (!plan) {
					if ((plan = generate_new_plan(signal_lengths[i])) == NULL) {
						moderror("Generating plan.\n");
						return -1;
					}
				}
				if (is_complex) {
					dft_run_c2r(plan, c_input, &f_pl_signals[i*INPUT_MAX_SAMPLES]);
				} else {
					dft_run_r2r(plan, r_input, &f_pl_signals[i*INPUT_MAX_SAMPLES]);
				}
				/*if (!is_complex) {
					signal_lengths[i] = signal_lengths[i]/2;
				}*/
				for (j=0;j<signal_lengths[i];j++) {
					pl_signals[i*INPUT_MAX_SAMPLES+j] = (double) f_pl_signals[i*INPUT_MAX_SAMPLES+j];
				}
			}
		}
		for (i=NOF_INPUT_ITF;i<2*NOF_INPUT_ITF;i++) {
			signal_lengths[i] = 0;
		}
		plp_draw(pl_signals,signal_lengths,0);

	break;
	default:
		moderror_msg("Unknown mode %d\n",mode);
		return -1;

	}
	return 0;
}

int stop() {
	if (plp_initiated) {
		moddebug("destoying plp %d\n",1);
		plp_destroy();
	}
	if (fft_initiated) {
		moddebug("destoying fft %d\n",1);
		dft_plan_free_vector(plans, NOF_PRECOMPUTED_DFT);
		dft_plan_free_vector(extra_plans, MAX_EXTRA_PLANS);
	}
	return 0;
}

void print_signal(void *input, int len) {
	int i;
	_Complex float *c_in = input;
	float *r_in = input;
	for (i=0;i<len;i++) {
		if (is_complex) {
			printf("input[%d/%d]=%.2f+i*%.2f\n",i, len,__real__ c_in[i],__imag__ c_in[i]);
		} else {
			printf("input[%d/%d]=%.2f\n",i, len,r_in[i]);
		}
	}
}

void setup_legends() {
	int i;

	for (i=0;i<NOF_INPUT_ITF;i++) {
		snprintf(r_legends[i],STR_LEN,"in%d",i);
		snprintf(c_legends[2*i],STR_LEN,"real(in%d)",i);
		snprintf(c_legends[2*i+1],STR_LEN,"imag(in%d)",i);
		snprintf(fft_legends[i],STR_LEN,"abs(in%d)",i);
	}
}



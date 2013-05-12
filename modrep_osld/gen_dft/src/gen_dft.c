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
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include <math.h>
#include <complex.h>

#include "dft/dft.h"
#include "gen_dft.h"

/** List of dft lengths (dft points) for which dft plans are precomputed during init */
const int precomputed_dft_len[] = {128, 72};
#define NOF_PRECOMPUTED_DFT 2

#define MAX_EXTRA_PLANS	5

pmid_t dft_size_id;

dft_plan_t plans[NOF_PRECOMPUTED_DFT];
dft_plan_t extra_plans[MAX_EXTRA_PLANS];
static int direction;
static int options;


#define MAX_DFT_SIZE	8192
#define twopi		6.28318530718
#define df_lte		7500

_Complex float precomputed_shift_reg[2048];
_Complex float precomputed_shift_1536[1536];
_Complex float computed_shift[MAX_DFT_SIZE];
_Complex float *shift;
int shift_increment;

pmid_t df_id, fs_id;
int previous_df, previous_fs, previous_dft_size;

void calculate_shift_vector(_Complex float *comp_shift, int df, int fs, int dft_size);
void precalculate_shift_vectors(_Complex float *shift_reg, _Complex float *shift_1536);
int process_shift_params(int df, int fs, int dft_size);

/**@ingroup Frequency shift vector calculation 
 * Calculates dft_size samples of the complex phasor for any freqeuncy shift df 
 * and sampling frequency fs.
 * \param comp_shift Pointer to samples of complex phasor (carrier)
 * \param df Frequency shift in Hz (positive for upconversion, negative for downconversion)
 * \param fs Sampling rate in Hz
 * \param dft_size Number of DFT/IDFT points
 */
void calculate_shift_vector(_Complex float *comp_shift, int df, int fs, int dft_size)
{
	int t;

	for (t=0;t<dft_size;t++) {
		comp_shift[t] = cos(twopi*df*t/fs) + _Complex_I*sin(twopi*df*t/fs);
	}
}

/**@ingroup Frequency shift vector precalculation
 * Calculates two complex phasors for a frequency shift of 7.5 kHz: 
 * 1) 1536 samples for sampling frequency 23.04 MHz (LTE 15 MHz UL mode) 
 * 2) 2048 samples for sampling frequency 30.72 MHz (LTE 20 MHz UL mode)
 * Note that 2) serves for the LTE UL modes 1.4, 3, 5, and 10 MHz.
 * \param shift_reg Pointer to 2048 samples of complex phasor at frequency 
 * df=7.5 kHz and a sampling rate fs2=30.72 MHz (2048-point fft, 20 MHz LTE mode)
 * \param shift_1536 Pointer to 1536 samples of complex phasor at frequency 
 * df=7.5 kHZ and a sampling rate fs1=23.04 MHz (1536-point fft, 15 MHz LTE mode)
 */
void precalculate_shift_vectors(_Complex float *shift_1536, _Complex float *shift_reg)
{
	int t;
	int fs1 = 23040000;	/* sampling rate for 15 MHz LTE RF Bw*/
	int fs2 = 30720000;	/* sampling rate for 20 MHz LTE RF Bw*/

	for (t=0;t<1536;t++) {
		shift_1536[t] = cos(twopi*df_lte*t/fs1) + _Complex_I*sin(twopi*df_lte*t/fs1);
	}
	for (t=0;t<2048;t++) {
		shift_reg[t] = cos(twopi*df_lte*t/fs2) + _Complex_I*sin(twopi*df_lte*t/fs2);
	}
}

/**@ingroup Process shift parameters
 * Processes the configuration parameters that define the shift pointer and index 
 * increment (shift_increment).
 * \param df Frequency shift in Hz (positive for upconversion, negative for downconversion)
 * \param fs Sampling rate in Hz
 * \param dft_size Number of DFT/IDFT points
 */
int process_shift_params(int df, int fs, int dft_size)
{
	if (df == df_lte) {
		if ((fs == 1920000) && (dft_size == 128)) {	/* 1.4 MHz LTE mode */
			shift = precomputed_shift_reg;
			shift_increment = 16;
		} else if ((fs == 3840000) && (dft_size == 256)) {/* 3 MHz LTE mode */
			shift = precomputed_shift_reg;
			shift_increment = 8;
		} else if ((fs == 7680000) && (dft_size == 512)) {/* 5 MHz LTE mode */
			shift = precomputed_shift_reg;
			shift_increment = 4;
		} else if ((fs == 15360000) && (dft_size == 1024)) {/* 10 MHz LTE mode */
			shift = precomputed_shift_reg;
			shift_increment = 2;
		} else if ((fs == 23040000) && (dft_size == 1536)) {/* 15 MHz LTE mode */
			shift = precomputed_shift_1536;
			shift_increment = 1;
		} else if ((fs == 30720000) && (dft_size == 2048)) {/* 20 MHz LTE mode */
			shift = precomputed_shift_reg;
			shift_increment = 1;
		} else {
			if (dft_size > MAX_DFT_SIZE) {
				moderror_msg("Too large DFT size %d. Maximum "
				"supported size is %d\n", dft_size, MAX_DFT_SIZE);
				return -1;
			}
			calculate_shift_vector(computed_shift, df, fs, dft_size);
			shift = computed_shift;
			shift_increment = 1;
		} 
	} else {
		if (dft_size > MAX_DFT_SIZE) {
			moderror_msg("Too large DFT size %d. Maximum supported size "
			"is %d\n", dft_size, MAX_DFT_SIZE);
			return -1;
		}
		calculate_shift_vector(computed_shift, df, fs, dft_size);
		shift = computed_shift;
		shift_increment = 1;
	}
	return 0;
}

/**@ingroup gen_dft
 * \param direction Direction of the dft: 0 computes a dft and 1 computes an idft (default is 0)
 * \param mirror 0 computes a normal dft, 1 swaps the two halfes of the input signal before computing
 * the dft (used in LTE) (default is 0)
 * \param dc_offset Set a null to the 0 subcarrier
 * \param psd Set to 1 to compute the power spectral density (untested) (default is 0)
 * \param out_db Set to 1 to produce the output results in dB (untested) (default is 0)
 * \param dft_size Number of DFT points. This parameter is mandatory.
 * \param df Frequency shift (choose a positive value for upconversion and a negative value for 
 * downconversion) (default is 0--no frequency shift)
 * \param fs Sampling rate. This parameter is mandatory if df!=0.
 */
int initialize() {
	int tmp;
	int i;

	memset(plans,0,sizeof(dft_plan_t)*NOF_PRECOMPUTED_DFT);
	memset(extra_plans,0,sizeof(dft_plan_t)*MAX_EXTRA_PLANS);

	if (param_get_int(param_id("direction"),&direction) != 1) {
		modinfo("Parameter direction not defined. Setting to FORWARD\n");
		direction = 0;
	}

	options = 0;
	if (param_get_int(param_id("mirror"),&tmp) != 1) {
		modinfo("Parameter mirror not defined. Disabling. \n");
	} else {
		if (tmp == 1) {
			options |= DFT_MIRROR_PRE;
		} else if (tmp == 2) {
			options |= DFT_MIRROR_POS;
		}
	}
	if (param_get_int(param_id("psd"),&tmp) != 1) {
		modinfo("Parameter psd not defined. Disabling. \n");
	} else {
		if (tmp) options |= DFT_PSD;
	}
	if (param_get_int(param_id("out_db"),&tmp) != 1) {
		modinfo("Parameter out_db not defined. Disabling. \n");
	} else {
		if (tmp) options |= DFT_OUT_DB;
	}
	if (param_get_int(param_id("dc_offset"),&tmp) != 1) {
		modinfo("Parameter dc_offset not defined. Disabling. \n");
	} else {
		if (tmp) options |= DFT_DC_OFFSET;
	}
	if (param_get_int(param_id("normalize"),&tmp) != 1) {
		modinfo("Parameter normalize not defined. Disabling. \n");
	} else {
		if (tmp) options |= DFT_NORMALIZE;
	}

	dft_size_id = param_id("dft_size");
	if (!dft_size_id) {
/*		moderror("Parameter dft_size not defined\n");
		return -1;*/
		modinfo("Parameter dft_size not defined. Assuming "
			"dft_size = number of input samples on interface 0.\n");
	}
		

	if (dft_plan_multi_c2c(precomputed_dft_len, (!direction)?FORWARD:BACKWARD, NOF_PRECOMPUTED_DFT,
			plans)) {
		moderror("Precomputing plans\n");
		return -1;
	}
	for (i=0;i<NOF_PRECOMPUTED_DFT;i++) {
		plans[i].options = options;
	}

	df_id = param_id("df");
	fs_id = param_id("fs");
	precalculate_shift_vectors(precomputed_shift_1536, precomputed_shift_reg);

	/* assume 1.4 MHz LTE UL Tx (fs = 1.92 MHz, 128 IFFT, df = 7.5 kHz) */
	shift = precomputed_shift_reg;
	previous_df = df_lte;
	previous_fs = 1920000;
	previous_dft_size = 128;
	shift_increment = 16;

	return 0;
}

dft_plan_t* find_plan(int dft_size) {
	int i;
	for (i=0;i<NOF_PRECOMPUTED_DFT;i++) {
		if (plans[i].size == dft_size) {
			return &plans[i];
		}
	}
	for (i=0;i<MAX_EXTRA_PLANS;i++) {
		if (extra_plans[i].size == dft_size) {
			return &plans[i];
		}
	}
	return NULL;
}

dft_plan_t* generate_new_plan(int dft_size) {
	int i;
	static int oldest_extra_plan = 0;

	modinfo_msg("Warning, no plan was precomputed for size %d. Generating.\n",dft_size);
	for (i=0;i<(MAX_EXTRA_PLANS-1);i++) {
		if (!extra_plans[i].size) {
			if (dft_plan_c2c(dft_size, (!direction)?FORWARD:BACKWARD, &extra_plans[i])) {
				return NULL;
			}
			extra_plans[i].options = options;
			return &extra_plans[i];
		}
	}

	if (oldest_extra_plan == (MAX_EXTRA_PLANS-1)) {
		oldest_extra_plan = 0;
	} else {
		oldest_extra_plan++;
	}
	if (dft_plan_c2c(dft_size, (!direction)?FORWARD:BACKWARD, &extra_plans[oldest_extra_plan])) {
		return NULL;
	}
	extra_plans[oldest_extra_plan].options = options;
	return &extra_plans[oldest_extra_plan];
	/*modinfo_msg("Maximum number of extra plans (%d) surpassed.\n",MAX_EXTRA_PLANS);
	return NULL;*/
}


int work(void **inp, void **out) {
	int i, j, k;
	int rcv_samples, nof_ffts;
	int df, fs;
	int e;
	int dft_size;
	input_t *input;
	output_t *output;
	dft_plan_t *plan;
	
 
	if (param_get_int(dft_size_id,&dft_size) != 1) {
		dft_size = get_input_samples(0);
		moddebug("Parameter dft_size not defined. Assuming %d"
		" (number of input samples on interface 0).\n", dft_size);
		/*moderror("Getting parameter dft_size\n");
		return -1;*/
	}

	if (dft_size == 0) {
		modinfo("dft_size = 0. Returning.\n");
		return 0;
	} else {
		moddebug("dft_size = %d.\n", dft_size);
	}
	
/*	if (param_get_int(param_id("dft_size"),&dft_size) != 1) {
		
		moddebug("Parameter dft_size not defined. Assuming %d"
			" (number of input samples on interface 0).\n", dft_size);
	}
*/
	plan = find_plan(dft_size);
	if (!plan) {
		if ((plan = generate_new_plan(dft_size)) == NULL) {
			moderror("Generating plan.\n");
			return -1;
		}
	}
	
	if (param_get_int(df_id, &df) != 1) {
		df = 0;
	}
	if (df != 0) {
		if (param_get_int(fs_id, &fs) != 1) {
			moderror("Parameter fs not defined.\n");
			return -1;
		}
		if (fs <= 0) {
			moderror("Sampling rate fs must be larger than 0.\n");
			return -1;
		}
		if ((df != previous_df) || (fs != previous_fs) || (dft_size != previous_dft_size)) {
			e = process_shift_params(df, fs, dft_size);
			if (e < 0) {
				return -1;
			}
			previous_df = df;
			previous_fs = fs;
			previous_dft_size = dft_size;
		}
	}

	for (i=0;i<NOF_INPUT_ITF;i++) {
		input = inp[i];
		output = out[i];
		rcv_samples = get_input_samples(i);
		moddebug("%d samples received on interface %d.\n",rcv_samples, i);
		if (rcv_samples == 0) {
			moddebug("%d samples to process. Returning.\n", rcv_samples);
			continue;
		}
		moddebug("Processing %d samples...\n",rcv_samples);

		if (rcv_samples % dft_size) {
			moderror_msg("Number of input samples (%d) not integer multiple"
			" of dft_size (%d) on interface %d\n",rcv_samples,dft_size,i);
			return -1;
		}

		nof_ffts = rcv_samples/dft_size;

		for (j=0;j<nof_ffts;j++) {
			if ((df != 0) && (direction == FORWARD)) { /* Rx: shift before FFT */
				for (k=0;k<dft_size;k++) {
					input[j*dft_size+k] *= shift[k*shift_increment];
				}
			}

			dft_run_c2c(plan, &input[j*dft_size], &output[j*dft_size]);

			if ((df !=0) && (direction == BACKWARD)) { /* Tx: shift after IFFT */
				for (k=0;k<dft_size;k++) {
					output[j*dft_size+k] *= shift[k*shift_increment];
				}
			}
		}

		set_output_samples(i,dft_size*nof_ffts);
		moddebug("%d samples sent to output interface %d.\n",dft_size*nof_ffts,i);
	}

	return 0;
}

int stop() {
	dft_plan_free_vector(plans, NOF_PRECOMPUTED_DFT);
	dft_plan_free_vector(extra_plans, MAX_EXTRA_PLANS);
	return 0;
}


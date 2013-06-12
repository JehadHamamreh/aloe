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
#include <skeleton.h>
#include <string.h>

#include "dft/dft.h"
#include "base/vector.h"
#include "lte_sss_synch.h"
#include "lte_lib/grid/base.h"

#include "find_sss.h"

int input_len;
dft_plan_t plan;

int sf_pm_idx;

struct sss_tables sss_tables;
int N_id_2,sss_pos_in_frame,sss_pos_in_symbol,correlation_threshold;

input_t input_fft[INPUT_MAX_SAMPLES];
int first;


int initialize() {

	input_len=0;
	param_get_int_name("input_len",&input_len);

	sss_pos_in_frame=960-2*137;
	param_get_int_name("sss_pos_in_frame",&sss_pos_in_frame);

	sss_pos_in_symbol=33;
	param_get_int_name("sss_pos_in_symbol",&sss_pos_in_symbol);

	correlation_threshold=3000;
	param_get_int_name("correlation_threshold",&correlation_threshold);

	N_id_2=0;
	param_get_int_name("N_id_2",&N_id_2);

	dft_plan(SSS_DFT_LEN,COMPLEX_2_COMPLEX,FORWARD,&plan);
	plan.options = DFT_MIRROR_POS | DFT_DC_OFFSET;

	generate_sss_all_tables(&sss_tables,N_id_2);
	convert_tables(&sss_tables);

#ifdef _COMPILE_ALOE
	sf_pm_idx = oesr_get_variable_idx(ctx, "ctrl","subframe_rx");
	if (sf_pm_idx < 0) {
		moderror("Error getting remote parameter subframe_rx\n");
	}
	moddebug("Remote subframe_rx parameter is a at %d\n",sf_pm_idx);
#endif

	first=1;
	return 0;
}

/**
 * @TODO:
 * - re-compute c from N_id_2
 */
int work(void **inp, void **out) {
	int rcv_samples=get_input_samples(0);
	int m0,m1;
	int subframe_idx,N_id_1;
	input_t *input=inp[0];

	if (!rcv_samples) {
		return 0;
	}

	dft_run_c2c(&plan, &input[sss_pos_in_frame], input_fft);

	if (!get_m0m1(&input_fft[sss_pos_in_symbol],&m0,&m1,correlation_threshold)) {
		return 0;
	}

	subframe_idx=decide_subframe(m0,m1);

	N_id_1=decide_N_id_1(m0,m1);

	modinfo_msg("m0=%d, m1=%d subframe=%d\n", m0,m1,subframe_idx,N_id_1);

#ifdef _COMPILE_ALOE

	if (sf_pm_idx >= 0 && !(subframe_idx && first)) {
		first=0;
		if (param_remote_set(out, 0, sf_pm_idx, &subframe_idx,
				sizeof(int))) {
			moderror("Setting parameter\n");
			return -1;
		}
	}
#endif

	return 0;
}



/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


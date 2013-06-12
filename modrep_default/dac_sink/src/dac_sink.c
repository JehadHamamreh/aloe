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
#include <oesr.h>
#include <rtdal.h>
#include <params.h>
#include <skeleton.h>
#include <complex.h>
#include <math.h>

#include "rtdal.h"

#include "dac_sink.h"

#include "base/vector.h"


char board[128],args[128];
pmid_t rate_id, gain_id, amp_id,freq_id;
r_dac_t dac;
static float amplitude,rate,gain,freq;
static int check_blen, enable_dac;
static int blocking;

complex_t out_buffer[INPUT_MAX_SAMPLES];

int process_params();

/**
 * @ingroup dac_sink
 *
 * \param rate Sets DA converter sampling rateuency
 */
int initialize() {
	var_t pm;
	pm = oesr_var_param_get(ctx, "board");
	if (!pm) {
		moderror("Parameter board undefined\n");
		return -1;
	}

	if (oesr_var_param_get_value(ctx, pm, board, 128) == -1) {
		moderror("Error getting board value\n");
		return -1;
	}
	pm = oesr_var_param_get(ctx, "args");
	if (pm) {
		if (oesr_var_param_get_value(ctx, pm, args, 128) == -1) {
			moderror("Error getting board value\n");
			return -1;
		}
	} else {
		bzero(args,128);
	}

	check_blen=0;
	param_get_int_name("check_blen",&check_blen);
	enable_dac=1;
	param_get_int_name("enable_dac",&enable_dac);

	rate_id = param_id("rate");
	if (!rate_id) {
		moderror("Parameter rate not found\n");
		return -1;
	}
	freq_id = param_id("freq");
	if (!freq_id) {
		moderror("Parameter freq not found\n");
		return -1;
	}
	gain_id = param_id("gain");
	amp_id = param_id("amplitude");

	blocking=0;
	param_get_int_name("blocking",&blocking);

	if (!enable_dac) {
		modinfo("Warning: DAC is disabled\n");
	}

	dac = rtdal_dac_open(board,args);
	return process_params();
}

int process_params() {
	float _rate,_gain,_freq;
	if (param_get_float(rate_id,&_rate) != 1) {
		modinfo("Getting parameter rate\n");
		return -1;
	}
	if (_rate != rate) {
		rate = _rate;
		_rate = rtdal_dac_set_tx_srate(dac,rate);
		modinfo_msg("Set TX sampling rate %g MHz\n", _rate/1000000);
	}

	if (param_get_float(freq_id,&_freq) != 1) {
		modinfo("Getting parameter freq\n");
		return -1;
	}
	if (_freq != freq) {
		freq = _freq;
		_freq = rtdal_dac_set_tx_freq(dac,freq);
		modinfo_msg("Set TX freq %g MHz\n",_freq/1000000);
	}

	if (gain_id) {
		if (param_get_float(gain_id,&_gain) != 1) {
			modinfo("Getting parameter gain\n");
			return -1;
		}
	} else {
		_gain = 0.0;
	}
	if (_gain != gain) {
		gain = _gain;
		_gain = rtdal_dac_set_tx_gain(dac,gain);
		modinfo_msg("Set TX gain %g dB (%g)\n",_gain,gain);
	}
	if (amp_id) {
		if (param_get_float(amp_id,&amplitude) != 1) {
			modinfo("Getting parameter amplitude\n");
			return -1;
		}
	} else {
		amplitude = 1.0;
	}
	return 0;
}

/**
 * @ingroup dac_sink
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	int rcv_samples;
	int n;

	rcv_samples = get_input_samples(0);
	if (!rcv_samples) {
		return 0;
	}
	if (check_blen && check_blen != rcv_samples) {
		moderror_msg("Expected %d samples but got %d\n", check_blen, rcv_samples);
		return -1;
	}
	if (process_params()) {
		return -1;
	}
	vec_mult_c_r((complex_t*) inp[0],out_buffer,amplitude,rcv_samples);
	if (enable_dac) {
		n = rtdal_dac_send(dac,out_buffer,rcv_samples,blocking);
	} else {
		n = rcv_samples;
	}
	modinfo_msg("send %d samples amplitude %g\n",n,amplitude);
	if (n != rcv_samples) {
		moderror_msg("Sent %d/%d samples\n",n,rcv_samples);
		return -1;
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


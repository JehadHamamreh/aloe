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

#include "dac_source.h"

#include "base/vector.h"


char board[128],args[128];
pmid_t rate_id, gain_id, freq_id, nsamples_id;
r_dac_t dac;
static float rate,gain,freq;
static int nsamples,wait_packets;
static int blocking;

int process_params();

/**
 * @ingroup dac_source
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

	blocking=0;
	param_get_int_name("blocking",&blocking);

	wait_packets=0;
	param_get_int_name("wait_packets",&wait_packets);

	nsamples_id = param_id("nsamples");
	if (!nsamples_id) {
		moderror("Parameter nsamples not found\n");
		return -1;
	}

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

	dac = rtdal_dac_open(board,args);
	if (!dac) {
		moderror_msg("Initiating DAC %s (args=%s)\n",board,args);
		return -1;
	}
	if (process_params()) {
		moderror("Setting parameters\n");
		return -1;
	}
	return 0;
}

int process_params() {
	float _rate,_gain,_freq;
	if (param_get_float(rate_id,&_rate) != 1) {
		modinfo("Getting parameter rate\n");
		return -1;
	}
	if (_rate != rate) {
		rate = _rate;
		_rate = rtdal_dac_set_rx_srate(dac,rate);
		modinfo_msg("Set TX sampling rate %g MHz\n", _rate/1000000);
	}

	if (param_get_float(freq_id,&_freq) != 1) {
		modinfo("Getting parameter freq\n");
		return -1;
	}
	if (_freq != freq) {
		freq = _freq;
		_freq = rtdal_dac_set_rx_freq(dac,freq);
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
		_gain = rtdal_dac_set_rx_gain(dac,gain);
		modinfo_msg("Set TX gain %g dB (%g)\n",_gain,gain);
	}
	param_get_int(nsamples_id, &nsamples);
	return 0;
}
int cnt=0;
int stream_started=0;
/**
 * @ingroup dac_source
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	int n;

	if (!stream_started) {
		rtdal_dac_start_rx_stream(dac);
		stream_started=1;
	}
	if (process_params()) {
		return -1;
	}
	if (!nsamples) {
		return 0;
	}
	if (!out[0]) {
		moderror("Output interface not ready\n");
		return -1;
	}
	if (cnt<wait_packets) {
		cnt++;
		modinfo_msg("not receiving ts=%d\n",oesr_tstamp(ctx));
		return 0;
	}
	n = rtdal_dac_recv(dac,out[0],nsamples,blocking);
	modinfo_msg("ts=%d, recv %d samples\n",oesr_tstamp(ctx),n);
	if (n != nsamples) {
		moderror_msg("Recv %d/%d samples\n",n,nsamples);
		return -1;
	}

	return nsamples;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


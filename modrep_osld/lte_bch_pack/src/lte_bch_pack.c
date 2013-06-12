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
#include <params.h>
#include <skeleton.h>
#include <string.h>

#include "base/pack.h"
#include "lte_bch_pack.h"

int direction;
extern int nof_input_itf;
extern int nof_output_itf;

int sfn_rx, nof_prb_rx;

void set_bw(char *output, int nof_prb) {

	if (nof_prb<=6) {
		output[0]=0;
		output[1]=0;
		output[2]=0;
	} else if (nof_prb<=15) {
		output[0]=0;
		output[1]=0;
		output[2]=1;
	} else if (nof_prb<=25) {
		output[0]=0;
		output[1]=1;
		output[2]=0;
	} else if (nof_prb<=50) {
		output[0]=0;
		output[1]=1;
		output[2]=1;
	} else if (nof_prb<=75) {
		output[0]=1;
		output[1]=0;
		output[2]=0;
	} else if (nof_prb<=100) {
		output[0]=1;
		output[1]=0;
		output[2]=1;
	}
}


int get_bw(char *input) {

	if (
		input[0]==0 &&
		input[1]==0 &&
		input[2]==0) {
		return 6;
	} else if (
		input[0]==0 &&
		input[1]==0 &&
		input[2]==1) {
		return 15;
	} else if (
		input[0]==0 &&
		input[1]==1 &&
		input[2]==0) {
		return 25;
	} else if (
		input[0]==0 &&
		input[1]==1 &&
		input[2]==1) {
		return 50;
	} else if (
		input[0]==1 &&
		input[1]==0 &&
		input[2]==0) {
		return 75;
	} else if (
		input[0]==1 &&
		input[1]==0 &&
		input[2]==1) {
		return 100;
	}
	return -1;
}

int send_bch(char *output) {
	int enable;
	int nof_prb,sfn;
	char *buffer;

	enable=0;
	param_get_int_name("enable",&enable);
	if (!enable) {
		return 0;
	}
	if (param_get_int_name("nof_prb",&nof_prb)) {
		modinfo("could not get parameter nof_prb\n");
	}
	if (param_get_int_name("sfn",&sfn)) {
		modinfo("could not get parameter sfn\n");
	}

	set_bw(output,nof_prb);
	output[3]=0; /* phich duration */
	output[4]=0; /* phich resources */
	output[5]=0;
	buffer=&output[6];
	pack_bits(sfn,&buffer,8);
	memset(&output[14],0,10); /* + 10 zeros */

#ifdef _COMPILE_ALOE
	moddebug("ts=%d transmitted nof_prb=%d, sfn=%d\n",oesr_tstamp(ctx),nof_prb,sfn);
#endif
	return 24;
}

int recv_bch(char *input, char *output, int len) {
#ifdef _COMPILE_ALOE
	int nof_prb,sfn,n;
	char *buffer;

	if (len < 24) {
		return 0;
	} else {
		nof_prb = get_bw(input);
		buffer=&input[6];
		/* get phich setup */
		sfn = unpack_bits(&buffer,8);
	}

	modinfo_msg("received nof_prb=%d sfn=%d\n",nof_prb,sfn);

	len = 0;
	n = param_remote_set_ptr(&output[len], nof_prb_rx, &nof_prb, sizeof(int));
	if (n == -1) {
		moderror("Setting parameter mcs\n");
		return -1;
	}
	len += n;
	n = param_remote_set_ptr(&output[len], sfn_rx, &sfn, sizeof(int));
	if (n == -1) {
		moderror("Setting parameter nof_rbg\n");
		return -1;
	}
	len += n;
	set_output_samples(0,len);
#endif

	return len;
}


/**
 * @ingroup lte_bch_pack
 * \param direction 0 for bch packing, 1 for bch unpacking
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	if (param_get_int_name("direction",&direction)) {
		moderror("Getting parameter direction\n");
		return -1;
	}

	if (!direction) {
		nof_input_itf = 0;
		nof_output_itf = 1;
	} else {
		nof_input_itf = 1;
		nof_output_itf = 1;
	}

#ifdef _COMPILE_ALOE
	nof_prb_rx = oesr_get_variable_idx(ctx, "ctrl","nof_prb_rx");
	sfn_rx = oesr_get_variable_idx(ctx, "ctrl","sfn_rx");
	if (nof_prb_rx < 0 || sfn_rx < 0) {
		moderror("Error getting remote parameters\n");
	}
#endif

	return 0;
}


int work(void **inp, void **out) {
	int len;

	if (!direction) {

		if (!out[0]) {
			return 0;
		}
		len = send_bch(out[0]);
		if (len == -1) {
			return -1;
		}
		return len;
	} else {
		len = get_input_samples(0);
		recv_bch(inp[0],out[0],len);
	}

	return 0;
}

int stop() {
	return 0;
}


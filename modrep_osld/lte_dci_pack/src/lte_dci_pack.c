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

#include "lte_dci_pack.h"
#include "dci_formats.h"

int direction;
extern int nof_input_itf;
extern int nof_output_itf;

int mcs_rx, nof_rbg_rx, rbg_mask_rx;


int send_dci(char *output) {
	struct dci_format1 dci; /* Only format1 is currently supported */
	int len;
	int i;
	int rbg_mask,enable;

	memset(&dci,0,sizeof(struct dci_format1));
	enable=0;
	param_get_int_name("enable",&enable);
	if (!enable) {
		return 0;
	}
	if (param_get_int_name("mcs",&dci.mcs)) {
		modinfo("could not get parameter mcs\n");
	}
	if (param_get_int_name("nof_rbg",&dci.nof_rbg)) {
		modinfo("could not get parameter nof_rbg\n");
	}
	rbg_mask=0;
	if (param_get_int_name("rbg_mask",&rbg_mask)) {
		modinfo("could not get parameter rbg_mask\n");
	}
	for (i=0;i<MAX_RBG_SET;i++) {
		if (rbg_mask & (0x1<<i)) {
			dci.rbg_mask[i] = 1;
		} else {
			dci.rbg_mask[i] = 0;
		}
	}
	dci.ra_type = NA; /* means type0 because nrb<10 */
	dci.harq_pnum_len = 3;
	dci.carrier_indicator_len=2; /* this is to make it divisible by 3 */

	len = dci_format1_pack(output,&dci);
	if (len < 0) {
		moderror("Building DCI Format 1 packet\n");
		return -1;
	}

#ifdef _COMPILE_ALOE
	moddebug("ts=%d transmitted mcs=%d, nof_rbg=%d, rbgmask=0x%x\n",oesr_tstamp(ctx),dci.mcs,dci.nof_rbg,rbg_mask);
#endif
	return len;
}

int recv_dci(char *input, char *output, int len) {
	struct dci_format1 dci; /* Only format1 is currently supported */
	int rbg_mask = 0;
	int i;

	if (len == 0) {
		return 0;
	} else {
		memset(&dci,0,sizeof(struct dci_format1));
		dci.harq_pnum_len = 3;
		dci.carrier_indicator_len=2;
		if (param_get_int_name("nof_rbg",&dci.nof_rbg)) {
			modinfo("could not get parameter nof_rbg\n");
		}
		if (dci_format1_unpack(input,len,&dci)<0) {
			moderror("Reading DCI Format 1 packet\n");
			return -1;
		}
		for (i=0;i<MAX_RBG_SET;i++) {
			rbg_mask |= (dci.rbg_mask[i] & 0x1) << (i);
		}
	}

#ifdef _COMPILE_ALOE
	len = 0;
	int n;
	n = param_remote_set_ptr(&output[len], mcs_rx, &dci.mcs, sizeof(int));
	if (n == -1) {
		moderror("Setting parameter mcs\n");
		return -1;
	}
	len += n;
	n = param_remote_set_ptr(&output[len], nof_rbg_rx, &dci.nof_rbg, sizeof(int));
	if (n == -1) {
		moderror("Setting parameter nof_rbg\n");
		return -1;
	}
	len += n;
	n = param_remote_set_ptr(&output[len], rbg_mask_rx, &rbg_mask, sizeof(int));
	if (n == -1) {
		moderror("Setting parameter rbg_mask\n");
		return -1;
	}
	len += n;
	set_output_samples(0,len);
	modinfo_msg("received mcs=%d, nof_rbg=%d, rbgmask=0x%x\n",dci.mcs,dci.nof_rbg,rbg_mask);
#endif

	return len;
}


/**
 * @ingroup lte_dci_pack
 * \param direction 0 for dci packing, 1 for dci unpacking
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
	mcs_rx = oesr_get_variable_idx(ctx, "ctrl","mcs_rx");
	nof_rbg_rx = oesr_get_variable_idx(ctx, "ctrl","nof_rbg_rx");
	rbg_mask_rx = oesr_get_variable_idx(ctx, "ctrl","rbg_mask_rx");
	if (mcs_rx < 0 || nof_rbg_rx < 0 || rbg_mask_rx < 0) {
		moderror("Error getting remote parameters\n");
	}
	modinfo_msg("remote params: mcs_rx=%d, nof_rbg_rx=%d, rbg_mask_rx=%d\n",mcs_rx,nof_rbg_rx,rbg_mask_rx);
#endif
	return 0;
}

int work(void **inp, void **out) {
	int len;

	if (!direction) {
		if (!out[0]) {
			return 0;
		}
		len = send_dci(out[0]);
		if (len == -1) {
			return -1;
		}
		return len;
	} else {
		len = get_input_samples(0);
		recv_dci(inp[0],out[0],len);
	}

	return 0;
}

int stop() {
	return 0;
}


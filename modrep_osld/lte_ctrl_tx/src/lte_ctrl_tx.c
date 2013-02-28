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
#include <skeleton.h>

#include "lte_ctrl_tx.h"

#define _DEFINE_VARIABLES_
#include "ctrl_logic.h"

extern int nof_output_itf;
pmid_t mcs_id,nrb_id;

static va_t addr[NOF_VARIABLES];
struct my_parameters old_pm;

/**
 * @ingroup lte_ctrl_tx
 *
 * \param fft_size
 * \param mcs Modulation and Coding Scheme
 * \param nrb Number of Resource Blocks
 *
 * \param nof_modules Equals the number of output interfaces
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	int i;

	if (param_get_int_name("nof_modules",&nof_output_itf)) {
		moderror("Parameter nof_modules undefined\n");
		return -1;
	}

	for (i=0;i<NOF_VARIABLES;i++) {
		addr[i].module_idx = oesr_get_module_idx(ctx, (char*) variables[i].module_name);
		if (addr[i].module_idx == -1) {
			moderror_msg("Module %s not found\n",variables[i].module_name);
			return -1;
		}
		addr[i].variable_idx = oesr_get_variable_idx(ctx, (char*) variables[i].module_name,
				(char*) variables[i].variable_name);
		if (addr[i].module_idx == -1) {
			moderror_msg("Variable %s not found in module %s\n",variables[i].variable_name,
					variables[i].module_name);
			return -1;
		}
	}
	i=0;
	while(myparams[i].name) {
		myparams[i].pmid = param_id((char*) myparams[i].name);
		if (!myparams[i].pmid) {
			moderror("Parameter mcs not found\n");
			return -1;
		}
		i++;
	}

	memset(&cur_pm,0,sizeof(struct my_parameters));
	memset(&old_pm,0,sizeof(struct my_parameters));

	return 0;
}

int my_parameters_update() {
	int i;
	i=0;
	while(myparams[i].name) {
		if (param_get(myparams[i].pmid, myparams[i].value, myparams[i].size, NULL) == -1) {
			return -1;
		}
		i++;
	}
	return 0;
}

int my_parameters_changed() {
	int n;
	n = memcmp(&old_pm,&cur_pm,sizeof(struct my_parameters));
	memcpy(&old_pm,&cur_pm,sizeof(struct my_parameters));
	return n;
}


int work(void **inp, void **out) {
	int tslot;

	tslot = oesr_tstamp(ctx);
	if (my_parameters_update()) {
		moderror("Uploading control parameters\n");
		return -1;
	}
	if (my_parameters_changed()) {
		ctrl_params_changed(tslot,&cur_pm);
	}

	ctrl_params_tslot(tslot,&cur_pm);

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


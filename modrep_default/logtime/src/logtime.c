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
#include <rtdal.h>

#include "logtime.h"

r_log_t logtime;
int log_size,bypass;

/**
 * @ingroup logtime
 *
 * Saves in a rtdal log the time (sec:usec) a packet passes through the module.
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {
	var_t pm;
	char name[64];
	pm = oesr_var_param_get(ctx, "file_name");
	if (!pm) {
		moderror("Parameter file_name undefined\n");
		return -1;
	}

	if (oesr_var_param_get_value(ctx, pm, name, 64) == -1) {
		moderror("Error getting file_name value\n");
		return -1;
	}
	bypass=0;
	param_get_int_name("bypass",&bypass);

	if (!bypass) {
		log_size=10000;
		param_get_int_name("log_size",&log_size);
		logtime = rtdal_log_new(name,UINT32,sizeof(unsigned int)*log_size);
		if (!logtime) {
			moderror("Creating log\n");
			return -1;
		}
	}
	return 0;
}

int cnt=0;

/**
 * @ingroup logtime
 *
 *
 */
int work(void **inp, void **out) {
	time_t t;
	unsigned int data[2];
	if (get_input_samples(0)) {
		if (cnt<log_size-100 && !bypass) {
			cnt++;
			rtdal_time_get(&t);
			data[0]=(unsigned int) t.tv_sec;
			data[1]=(unsigned int) t.tv_usec;
			rtdal_log_add(logtime,data,2*sizeof(unsigned int));
		}
		memcpy(out[0],inp[0],get_input_samples(0));
		//modinfo_msg("received %d bytes\n",get_input_samples(0));
		return get_input_samples(0);
	}
	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


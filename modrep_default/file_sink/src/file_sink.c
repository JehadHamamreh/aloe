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
#include <rtdal_datafile.h>
#include <params.h>
#include <skeleton.h>
#include <complex.h>
#include "file_sink.h"

FILE *fd;

char buffer[INPUT_MAX_SAMPLES];
static int rpm, buffer_sz,last_rcv_samples;
static int data_type;

extern int input_sample_sz;

/**
 * @ingroup file_sink
 *
 * Waits to receive buffer_sz samples and then writes the signal to file_name.
 * When buffer_sz new samples are received, the signal is overwritten.
 *
 *	If buffer_sz is not provided, the signal is saved each timeslot.
 *
 * \param data_type 0 for float, 1 for _Complex float and 2 for _Complex short
 * \param file_name Name of the *.mat file to save the signal
 * \param buffer_sz Number of samples to buffer before writing to file.
 */
int initialize() {
	char name[64];
	var_t pm;
	int i;

	if (param_get_int_name("data_type", &data_type)) {
		moderror("Parameter data_type undefined\n");
		return -1;
	}

	switch(data_type) {
	case 0:
		input_sample_sz = sizeof(float);
		break;
	case 1:
		input_sample_sz = sizeof(_Complex float);
		break;
	case 2:
		input_sample_sz = sizeof(_Complex short);
		break;
	}

	if (param_get_int_name("buffer_sz",&buffer_sz)) {
		modinfo("buffer_sz undefined. Saving data each timeslot\n");
		buffer_sz=0;
	}

	if (buffer_sz > INPUT_MAX_SAMPLES) {
		moderror_msg("Maximum buffer size is %d\n",INPUT_MAX_SAMPLES);
		return -1;
	}

#ifdef _COMPILE_ALOE
	pm = oesr_var_param_get(ctx, "file_name");
	if (!pm) {
		moderror("Parameter file_name undefined\n");
		return -1;
	}

	if (oesr_var_param_get_value(ctx, pm, name, 64) == -1) {
		moderror("Error getting file_name value\n");
		return -1;
	}
#endif

	modinfo_msg("Opening mat-file %s for writing\n",name);
	fd = rtdal_datafile_open(name,"w");
	if (!fd) {
		moderror_msg("Error opening file %s\n",name);
		return -1;
	}
	rpm = 0;
	last_rcv_samples = 0;

	return 0;
}

/**
 * @ingroup file_sink
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	int rcv_samples;
	input_t *input;

	rcv_samples = get_input_samples(0);

#ifdef _COMPILE_ALOE
	if (rcv_samples != last_rcv_samples) {
		last_rcv_samples = rcv_samples;
		modinfo_msg("Receiving %d samples at tslot %d\n",
				rcv_samples,oesr_tstamp(ctx));
	}
#endif

	memcpy(&buffer[rpm*input_sample_sz],inp[0],input_sample_sz*rcv_samples);
	rpm += rcv_samples;
	if (rpm >= buffer_sz) {
		switch(data_type) {
		case 0:
			rtdal_datafile_write_real(fd,(float*) buffer,rpm);
			break;
		case 1:
			rtdal_datafile_write_complex(fd,
					(_Complex float*) buffer,rpm);
			break;
		case 2:
			rtdal_datafile_write_complex_short(fd,
					(_Complex short*) buffer,rpm);
			break;
		}
		rpm = 0;
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


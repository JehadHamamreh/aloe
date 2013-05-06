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
#include "file_source.h"

FILE *fd;

#ifdef _COMPILE_ALOE
static int last_snd_samples;
#endif
static int data_type;
static int block_length;

extern int output_sample_sz;

char buffer[153600];

/**
 * @ingroup file_source
 *
 * \param data_type If specified, reads formated data in text mode:
 * 0 for float, 1 for _Complex float and 2 for _Complex short
 * \param block_length Number of samples to read per timeslot, or bytes
 * \param file_name Name of the *.mat file to save the signal
 */
int initialize() {
	char name[64];

	if (param_get_int_name("data_type", &data_type)) {
		data_type=-1;
	}

	switch(data_type) {
	case 0:
		output_sample_sz = sizeof(float);
		break;
	case 1:
		output_sample_sz = sizeof(_Complex float);
		break;
	case 2:
		output_sample_sz = sizeof(_Complex short);
		break;
	case -1:
		output_sample_sz=sizeof(char);
	}

	if (param_get_int_name("block_length", &block_length)) {
		moderror("Parameter block_length not specified\n");
		return -1;
	}

#ifdef _COMPILE_ALOE
	var_t pm;
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

	modinfo_msg("Opening file %s for reading\n",name);
	fd = rtdal_datafile_open(name,"r");
	if (!fd) {
		moderror_msg("Error opening file %s\n",name);
		return -1;
	}

	int n = rtdal_datafile_read_bin(fd,buffer,10*block_length);
	printf("read %d bytes\n",n);
	return 0;
}
static int cnt=0;
/**
 * @ingroup file_source
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	output_t *output = out[0];

/*
	switch(data_type) {
	case 0:
		n = rtdal_datafile_read_real(fd,(float*) output,block_length);
		break;
	case 1:
		n = rtdal_datafile_read_complex(fd,
				(_Complex float*) output,block_length);
		break;
	case 2:
		n = rtdal_datafile_read_complex_short(fd,
				(_Complex short*) output,block_length);
		break;
	case -1:
		n = rtdal_datafile_read_bin(fd,output,block_length);
	}
*/
#ifdef _COMPILE_ALOE
	int n;
	n=block_length;
	if (n != last_snd_samples) {
		last_snd_samples = n;
		modinfo_msg("Sending %d samples at tslot %d\n",
				n,oesr_tstamp(ctx));
	}
#endif
	
	memcpy(output,&buffer[cnt*block_length],block_length);
	cnt++;
	if (cnt==10) {
		cnt=0;
	}
	
	return block_length;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


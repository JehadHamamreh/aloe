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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oesr.h>
#include <params.h>
#include <skeleton.h>

#include "block_adapter.h"

#define DEFAULT_BUFFER_SZ 100*1024
pmid_t blen_id;
int buffer_size;
int rpm,wpm;
char *buffer=NULL;

/**
 * @ingroup block_adapter
 *
 * \param buffer_size Size of the internal buffer
 * \param block_length Output packet length
 *
 * \returns This function returns 0 on success or -1 on error
 */
int initialize() {

	blen_id = param_id("block_length");

	buffer_size = 0;
	param_get_int_name("buffer_size",&buffer_size);
	if (!buffer_size) {
		buffer_size = DEFAULT_BUFFER_SZ;
	}
	buffer = malloc(buffer_size);
	if (!buffer) {
		moderror_msg("allocating buffer size %d\n",buffer_size);
		return -1;
	}
	rpm=0;
	wpm=0;

	return 0;
}

int buffer_data() {
	if (wpm >= rpm) {
		return wpm-rpm;
	} else {
		return buffer_size-(rpm-wpm);
	}
}

int buffer_space() {
	return buffer_size-buffer_data();
}

void put_data(char *data, int len) {
	int hlen;

	if (wpm+len < buffer_size) {
		memcpy(&buffer[wpm],data,len);
		wpm += len;
	} else {
		hlen = buffer_size-wpm;
		memcpy(&buffer[wpm],data,hlen);
		memcpy(buffer,&data[hlen],len-hlen);
		wpm = len-hlen;
	}
}

void get_data(char *data, int len) {
	int hlen;

	if (rpm+len < buffer_size) {
		memcpy(data,&buffer[rpm],len);
		rpm += len;
	} else {
		hlen = buffer_size - rpm;
		memcpy(data,&buffer[rpm],hlen);
		memcpy(&data[hlen],buffer,len-hlen);
		rpm = len-hlen;
	}
}

/**
 * @ingroup block_adapter
 *
 */
int work(void **inp, void **out) {
	input_t *input = inp[0];
	output_t *output = out[0];
	int rcv_samples = get_input_samples(0);
	int block_length;

	if (rcv_samples) {
		if (rcv_samples < buffer_space()) {
			modinfo_msg("put %d samples r=%d w=%d data=%d space=%d\n",rcv_samples,rpm,wpm,buffer_data(),buffer_space());
			put_data(input,rcv_samples);
		} else {
			modinfo_msg("Buffer full. Recv=%d bytes, space=%d bytes\n",
					rcv_samples,buffer_space());
		}
	} else {
		modinfo("received no data\n");
	}

	block_length=0;
	param_get_int(blen_id,&block_length);
	/* Verify control parameters */
	if (block_length > output_max_samples || block_length < 0) {
		moderror_msg("Invalid block length %d\n", block_length);
		return -1;
	}

	if (buffer_data() > block_length) {
		modinfo_msg("get %d samples r=%d w=%d data=%d space=%d\n",block_length,rpm,wpm,buffer_data(),buffer_space());
		get_data(output,block_length);
		return block_length;
	} else {
		return 0;
	}
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	if (buffer) {
		free(buffer);
	}
	return 0;
}


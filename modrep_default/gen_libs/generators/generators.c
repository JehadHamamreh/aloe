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
#include <complex.h>

#define RANDOM_NUMBERS	(1024*16)
#define RANDOM_BITS		4*RANDOM_NUMBERS

#define DIV(a,b) ((a-1)/b+1)

static int tmp_random[RANDOM_NUMBERS];
static char tmp_binary[RANDOM_BITS];

#define BPSK_SYMB 0.707

static int cnt = 0;

#define RAND 0
#define FIXED 1
#define SEQ 2

#define TYPE RAND

void generator_init_random() {
	int i;
	for (i=0;i<RANDOM_NUMBERS;i++) {
		tmp_random[i] = rand();
	}
	cnt = 0;
}

int* random_sequence(int len) {
	cnt += len;
	if (cnt >= RANDOM_NUMBERS)
		cnt -= RANDOM_NUMBERS;

	return &tmp_random[cnt];
}

void random_bits(char *output, int len) {
	int j,b,k,tmp;
	int *ptr;
	ptr = random_sequence(DIV(len,sizeof(int)*8));
	b=k=0;
	for (j=0;j<len;j++) {
		if (!b) {
			tmp = ptr[k];
			b = sizeof(int)*8;
			k++;
		}
		if (tmp & 0x1) {
			output[j] = 1;
		} else {
			output[j] = 0;
		}
		tmp>>=1;
		b--;
	}
}

int work_binary(char *output, int block_size) {
	int i,j;

	for (j=0;j<block_size;j++) {
		switch(TYPE) {
		case RAND:
			output[j] = rand()%2;
			break;
		case FIXED:
			output[j] = j%2;
			break;
		case SEQ:
			random_bits(tmp_binary,block_size);
			output[j] = tmp_binary[j];
			break;
		}
	}
	return block_size;
}

int work_ramp_re(float *output, int block_size) {
	int i,j;

	for (j=0;j<block_size;j++) {
		output[j] = i;
	}
	return block_size*sizeof(float);
}

int work_ramp_c(_Complex float *output, int block_size) {
	int i,j;

	for (j=0;j<block_size;j++) {
		__real__ output[j] = j;
		__imag__ output[j] = block_size-j-1;
	}
	return block_size*sizeof(_Complex float);
}

int work_bpsk_re(float *output, int block_size) {
	int i,j;

	random_bits(tmp_binary,block_size);
	for (j=0;j<block_size;j++) {
		if (tmp_binary[j]) {
			output[j] = BPSK_SYMB;
		} else {
			output[j] = -BPSK_SYMB;
		}
	}
	return block_size*sizeof(float);
}

int work_bpsk_c(_Complex float *output, int block_size) {
	int i,j;

	random_bits(tmp_binary,block_size);
	for (j=0;j<block_size;j++) {
		if (tmp_binary[j]) {
			__real__ output[j] = BPSK_SYMB;
		} else {
			__real__ output[j] = -BPSK_SYMB;
		}
	}
	random_bits(tmp_binary,block_size);
	for (j=0;j<block_size;j++) {
		if (tmp_binary[j]) {
			__imag__ output[j] = BPSK_SYMB;
		} else {
			__imag__ output[j] = -BPSK_SYMB;
		}
	}
	return block_size*sizeof(_Complex float);
}

int work_sin_re(float *output, int block_size) {
	int i,j;
	int cnt=0;
	for (j=0;j<block_size;j++) {
		switch(cnt) {
		case 0:
			output[j] = 1;
		break;
		case 1:
			output[j] = 0;
		break;
		case 2:
			output[j] = -1;
		break;
		case 3:
			output[j] = 0;
		break;
		}
		cnt++;
		if (cnt == 4) {
			cnt = 0;
		}
	}
	return block_size*sizeof(float);
}

int work_sin_c(_Complex float *output, int block_size) {
	int i,j;
	int cnt=0;
	for (j=0;j<block_size;j++) {
		switch(cnt) {
		case 0:
			__real__ output[j] = 1;
			__imag__ output[j] = 0;
		break;
		case 1:
			__real__ output[j] = 0;
			__imag__ output[j] = 1;
		break;
		case 2:
			__real__ output[j] = -1;
			__imag__ output[j] = 0;
		break;
		case 3:
			__real__ output[j] = 0;
			__imag__ output[j] = -1;
		break;
		}
		cnt++;
		if (cnt == 4) {
			cnt = 0;
		}
	}
	return block_size*sizeof(_Complex float);
}


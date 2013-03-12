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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "rtdal_datafile.h"

#include "skeleton.h"
#include "params.h"
#include "gnuplot_i.h"

FILE *dat_input=NULL, *dat_output=NULL;
char *dat_input_name=NULL, *dat_output_name=NULL;

extern const int input_sample_sz;
extern const int output_sample_sz;
extern const int nof_input_itf;
extern const int nof_output_itf;
extern const int input_max_samples;
extern const int output_max_samples;


static int *input_lengths;
static int *output_lengths;

static void **input_ptr, **output_ptr;

static char *input_data, *output_data;

int use_gnuplot;

typedef struct {
	char *name;
	char *value;
} saparam_t;

saparam_t *parameters;
int nof_params;


int parse_paramters(int argc, char**argv);

inline int get_input_samples(int idx) {
	if (idx<0 || idx>nof_input_itf)
		return -1;
	return input_lengths[idx];
}

inline int set_output_samples(int idx, int len) {
	if (idx<0 || idx>nof_output_itf)
			return -1;
	output_lengths[idx] = len;
	return 0;
}

void allocate_memory() {
	input_data = calloc(input_sample_sz,input_max_samples*nof_input_itf);
	assert(input_data);
	output_data = calloc(output_sample_sz,output_max_samples*nof_output_itf);
	assert(output_data);
	input_lengths = calloc(sizeof(int),nof_input_itf);
	assert(input_lengths);
	output_lengths = calloc(sizeof(int),nof_output_itf);
	assert(output_lengths);
	input_ptr = calloc(sizeof(void*),nof_input_itf);
	assert(input_ptr);
	output_ptr = calloc(sizeof(void*),nof_output_itf);
	assert(output_ptr);
}

void free_memory() {
	free(input_data);
	free(output_data);
	free(input_lengths);
	free(output_lengths);
	if (parameters) {
		for (int i=0;i<nof_params;i++) {
			if (parameters[i].name) free(parameters[i].name);
			if (parameters[i].value) free(parameters[i].value);
		}
		free(parameters);
	}
}

int get_time(struct timespec *x) {

	if (clock_gettime(CLOCK_MONOTONIC,x)) {
		return -1;
	}
	return 0;
}

void get_time_interval(struct timespec * tdata) {

	tdata[0].tv_sec = tdata[2].tv_sec - tdata[1].tv_sec;
	tdata[0].tv_nsec = tdata[2].tv_nsec - tdata[1].tv_nsec;
	if (tdata[0].tv_nsec < 0) {
		tdata[0].tv_sec--;
		tdata[0].tv_nsec += 1000000000;
	}
}

/**
 * Testsuite:
 * Generates random symbols and calls the module ....
 * @param ...
 * @param ...
 * @param ... */
int main(int argc, char **argv)
{
	struct timespec tdata[3];
	gnuplot_ctrl *plot;
	char tmp[64];
	int ret, i, j;
	float *tmp_f;
	_Complex float *tmp_c;
	double *plot_buff_r;
	double *plot_buff_c;
	int run_times;
	int file_read_sz;

	parameters = NULL;

	parse_paramters(argc, argv);

	run_times=1;
	if (param_get(param_id("run_times"),&run_times,sizeof(int),NULL) != sizeof(int)) {
		run_times=1;
	}

	if (initialize()) {
		printf("Error initializing\n");
		exit(1); /* the reason for exiting should be printed out beforehand */
	}

	allocate_memory();


	if (dat_input_name) {
		dat_input = rtdal_datafile_open(dat_input_name, "r");
		if (!dat_input) {
			printf("Error opening mat file %s\n",dat_input_name);
			exit(1);
		}
	}
	if (dat_output_name) {
		dat_output = rtdal_datafile_open(dat_output_name, "w");
		if (!dat_output) {
			printf("Error opening mat file %s\n",dat_output_name);
			exit(1);
		}
	}

	if (dat_input) {
		if (param_get_int_name("block_length",&file_read_sz)) {
			file_read_sz=input_max_samples;
		}
		if (input_sample_sz == sizeof(float)) {
			input_lengths[0] = rtdal_datafile_read_real(dat_input,
					(float*) input_data,file_read_sz);
		} else if (input_sample_sz == sizeof(_Complex float)) {
			input_lengths[0] = rtdal_datafile_read_complex(dat_input,
					(_Complex float*) input_data,file_read_sz);
		} else {
			printf("Only real and complex signals are supported\n");
		}
		if (input_lengths[0] == -1) {
			printf("Error reading file %s\n",dat_input_name);
			exit(1);
		}
		printf("Read %d/%d samples from file %s %d\n",input_lengths[0],file_read_sz,dat_input_name);
	} else {
		if (generate_input_signal(input_data, input_lengths)) {
			printf("Error generating input signal\n");
			exit(1);
		}
	}

	for (i=0;i<nof_input_itf;i++) {
		if (!input_lengths[i]) {
			printf("Warning input interface %d has zero length\n",i);
		}
	}

	for (i=0;i<nof_input_itf;i++) {
		input_ptr[i] = &input_data[i*input_max_samples*input_sample_sz];
	}
	for (i=0;i<nof_output_itf;i++) {
		output_ptr[i] = &output_data[i*output_max_samples*output_sample_sz];
	}
	ret = 0;
	clock_gettime(CLOCK_MONOTONIC,&tdata[1]);
	for (i=0;i<run_times;i++) {
		ret = work(input_ptr, output_ptr);
	}
	clock_gettime(CLOCK_MONOTONIC,&tdata[2]);

	stop();
	if (ret == -1) {
		printf("Error running\n");
		exit(-1);
	}
	get_time_interval(tdata);

	for (i=0;i<nof_output_itf;i++) {
		if (!output_lengths[i]) {
			output_lengths[i] = ret;
		}
		if (!output_lengths[i]) {
			printf("Warning output interface %d has zero length\n",i);
		} else {
			printf("output %d has %d samples\n",i,output_lengths[i]);
		}
	}

	if (dat_output) {
		if (output_sample_sz == sizeof(float)) {
			rtdal_datafile_write_real(dat_output,
					(float*) output_data,output_lengths[0]);
		} else if (input_sample_sz == sizeof(_Complex float)){
			rtdal_datafile_write_complex(dat_output,
					(_Complex float*) output_data,output_lengths[0]);
		} else {
			printf("Only real and complex signals are supported\n");
		}
	}

	printf("\nExecution time: %d ns.\n", (int) tdata[0].tv_nsec);
	printf("FINISHED\n");

	if (dat_output)
		rtdal_datafile_close(dat_output);
	if (dat_input)
		rtdal_datafile_close(dat_input);


	if (use_gnuplot) {
		for (i=0;i<nof_input_itf;i++) {
			plot_buff_r = malloc(sizeof(double)*input_lengths[i]);
			plot_buff_c = malloc(sizeof(double)*input_lengths[i]);
			if (input_sample_sz == sizeof(float)) {
				tmp_f = (float*) &input_data[i*input_max_samples*input_sample_sz];
				for (j=0;j<input_lengths[i];j++) {
					plot_buff_r[j] = (double) tmp_f[j];
				}
				plot = gnuplot_init() ;
			    gnuplot_setstyle(plot,"lines");
			    snprintf(tmp,64,"input_%d",i);
		        gnuplot_plot_x(plot, plot_buff_r,
		        		get_input_samples(i), tmp);
		        free(plot_buff_r);
			} else if (input_sample_sz == sizeof(_Complex float)) {
				tmp_c = (_Complex float*) &input_data[i*input_max_samples*input_sample_sz];
				for (j=0;j<input_lengths[i];j++) {
					plot_buff_r[j] = (double) __real__ tmp_c[j];
					plot_buff_c[j] = (double) __imag__ tmp_c[j];
				}
				plot = gnuplot_init() ;
			    gnuplot_setstyle(plot,"lines");
			    snprintf(tmp,64,"input_real_%d",i);
		        gnuplot_plot_x(plot, plot_buff_r,
		        		get_input_samples(i), tmp);
		        plot = gnuplot_init() ;
			    gnuplot_setstyle(plot,"lines");
			    snprintf(tmp,64,"input_imag_%d",i);
		        gnuplot_plot_x(plot, plot_buff_c,
		        		get_input_samples(i), tmp);
		        free(plot_buff_r);
		        free(plot_buff_c);
			}
		}
		for (i=0;i<nof_output_itf;i++) {
			plot_buff_r = malloc(sizeof(double)*output_lengths[i]);
			plot_buff_c = malloc(sizeof(double)*output_lengths[i]);
			if (output_sample_sz == sizeof(float)) {
				tmp_f = (float*) &output_data[i*output_max_samples*output_sample_sz];
				for (j=0;j<output_lengths[i];j++) {
					plot_buff_r[j] = (double) __real__ tmp_f[j];
				}
				plot = gnuplot_init() ;
				gnuplot_setstyle(plot,"lines");
				snprintf(tmp,64,"output_%d",i);
				gnuplot_plot_x(plot, plot_buff_r,
						output_lengths[i], tmp);
				free(plot_buff_r);
			} else if (output_sample_sz == sizeof(_Complex float)) {
				tmp_c = (_Complex float*) &output_data[i*output_max_samples*output_sample_sz];
				for (j=0;j<output_lengths[i];j++) {
					plot_buff_r[j] = (double) __real__ tmp_c[j];
					plot_buff_c[j] = (double) __imag__ tmp_c[j];
				}
				plot = gnuplot_init() ;
				gnuplot_setstyle(plot,"lines");
				snprintf(tmp,64,"output_real_%d",i);
				gnuplot_plot_x(plot, plot_buff_r,
						output_lengths[i], tmp);
				plot = gnuplot_init() ;
				gnuplot_setstyle(plot,"lines");
				snprintf(tmp,64,"output_imag_%d",i);
				gnuplot_plot_x(plot, plot_buff_c,
						output_lengths[i], tmp);
				free(plot_buff_r);
				free(plot_buff_c);
	        }
		}

		printf("Type ctrl+c to exit\n");fflush(stdout);
		free_memory();
		pause();
		/* make sure we exit here */
		exit(1);
	}
	free_memory();

	return 0;
}

pmid_t param_id(char *name) {
	int i;
	for (i=0;i<nof_params;i++) {
		if (!strcmp(name,parameters[i].name))
			break;
	}
	if (i==nof_params) {
		return NULL;
	}
	return &parameters[i];
}

int param_get(pmid_t id, void *ptr, int max_size, param_type_t *type) {
	int itmp;
	float ftmp;
	saparam_t *param = (saparam_t*) id;

	if (!ptr) return -1;
	if (!max_size) return -1;
	if (!id) return -1;


	/**@TODO: Support comma-separated parameter values */

	/* parse value */

	/* float */
	if (index(param->value,'.') || index(param->value,',')) {
		if (sscanf(param->value,"%f",&ftmp) != 0) {
			if (max_size < sizeof(float))
				return -1;

			*((float*) ptr) = ftmp;
			if (type) {
				*type = FLOAT;
			}
			return sizeof(float);
		}
		return -1;
	}

	/* is integer */
	/* try first hexadecimal */
	if (index(param->value,'x') || index(param->value,'X')) {
		if (sscanf(param->value,"0x%x",&itmp) != 0) {
			if (max_size < sizeof(int))
				return -1;

			*((int*) ptr) = itmp;
			if (type) {
				*type = INT;
			}
			return sizeof(int);
		}
	}
	/* else is integer */
	if (sscanf(param->value,"%d",&itmp) != 0) {
		if (max_size < sizeof(int))
			return -1;

		*((int*) ptr) = itmp;
		if (type) {
			*type = INT;
		}
		return sizeof(int);
	}

	/* else assume is a string */
	strncpy(ptr,param->value,max_size);
	if (type) {
		*type = STRING;
	}
	return strnlen(ptr,max_size);
}


/* Define test environment functions here */
int parse_paramters(int argc, char**argv)
{
	int i;
	char *key,*value;
	int k = 0;

	use_gnuplot = 0;

	nof_params = argc-1;
	for (i=1;i<argc;i++) {
		if (!strcmp(argv[i],"-p")) {
			use_gnuplot = 1;
			nof_params--;
		} else if (!strcmp(argv[i],"-i")) {
			dat_input_name = argv[i+1];
			i++;
			nof_params-=2;
		} else if (!strcmp(argv[i],"-o")) {
			dat_output_name = argv[i+1];
			i++;
			nof_params-=2;
		}
	}

	if (!nof_params) {
		return 0;
	}

	parameters = calloc(sizeof(saparam_t),nof_params);

	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-p")) {
			key = argv[i];
			value = index(argv[i],'=');
			if (value) {
				*value = '\0';
				value++;
				parameters[k].name = strdup(key);
				parameters[k].value = strdup(value);
				k++;
			}
		}
	}
	return 0;
}


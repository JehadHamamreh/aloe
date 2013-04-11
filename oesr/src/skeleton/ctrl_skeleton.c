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
#include <params.h>

#include "ctrl_pkt.h"
#include "ctrl_skeleton.h"

#define MAX_OUTPUTS 100
#define MAX_INPUT_PACKETS	20

typedef struct {
	int module_idx;
	itf_t itf;
}mod_addr_t;

typedef struct {
	int module_idx;
	int variable_idx;
	mod_addr_t *addr;
}pm_addr_t;

extern const int ctrl_send_always;

void *ctx;

itf_t ctrl_in;

extern remote_params_db_t remote_params_db[];
extern my_params_db_t local_params_db[];

static mod_addr_t outputs[MAX_OUTPUTS];
static pm_addr_t remote_variables[MAX_VARIABLES];
static var_t local_variables[MAX_VARIABLES];

static int nof_remote_variables, nof_local_variables, nof_remote_itf;

static struct ctrl_in_pkt ctrl_in_buffer[MAX_INPUT_PACKETS], ctrl_out_buffer;

static int nof_output_data_itf;

void init_memory() {
	memset(outputs,0,MAX_OUTPUTS*sizeof(mod_addr_t));
	memset(remote_variables,0,MAX_VARIABLES*sizeof(pm_addr_t));
	memset(local_variables,0,MAX_VARIABLES*sizeof(pmid_t));
}

int kkts=0;

int ctrl_skeleton_send_idx(int dest_idx, void *value, int size,int tstamp) {
	int n;
	if (dest_idx<0 || dest_idx>nof_remote_variables) {
		return -1;
	}
	if (remote_variables[dest_idx].module_idx == 77) {
		n=0;
	}
	ctrl_out_buffer.pm_idx = remote_variables[dest_idx].variable_idx;
	ctrl_out_buffer.size = size;
	memcpy(ctrl_out_buffer.value,value,size);

	n=oesr_itf_write(remote_variables[dest_idx].addr->itf,&ctrl_out_buffer,size + CTRL_PKT_HEADER_SZ,tstamp);
	if (n == -1) {
		return -1;
	} else if (!n) {
		printf("Buffer full while sending control packet to %s:%s\n",remote_params_db[dest_idx].module_name,
				remote_params_db[dest_idx].variable_name);
		return -1;
	}
	return 0;
}

int ctrl_skeleton_send_name(char *module_name, char *variable_name, void *value, int size,int tstamp) {
	int i;
	i=0;
	while(i<nof_remote_variables && strcmp(remote_params_db[i].module_name,module_name)
		&& strcmp(remote_params_db[i].variable_name,variable_name)) {
		i++;
	}
	if (i==nof_remote_variables) {
		return -1;
	}
	return ctrl_skeleton_send_idx(i,value,size,tstamp);
}

int remote_parameters_sendall(void *ctx) {
	int i;
	for (i=0;i<nof_remote_variables;i++) {
		if (ctrl_skeleton_send_idx(i, remote_params_db[i].value, remote_params_db[i].size,oesr_tstamp(ctx))) {
			moderror_msg("sending ctrl packet to %s:%s\n",
					remote_params_db[i].module_name,remote_params_db[i].variable_name);
			return -1;
		}
	}
	kkts++;
	return 0;
}

int init_ctrl_input(void *ctx) {

	ctrl_in = oesr_itf_create(ctx, 0, ITF_READ, sizeof(struct ctrl_in_pkt));
	if (ctrl_in == NULL) {
		if (oesr_error_code(ctx) == OESR_ERROR_NOTREADY) {
			return 0;
		} else {
			return -1;
		}
	} else {
		modinfo("Created control port\n");
	}
	return 1;
}

int init_remote_variables(void *ctx) {
	int i;

	i=0;
	while(remote_params_db[i].module_name && i<MAX_VARIABLES) {
		remote_variables[i].module_idx = oesr_get_module_idx(ctx, (char*) remote_params_db[i].module_name);
		if (remote_variables[i].module_idx == -1) {
			moderror_msg("Module %s not found\n",remote_params_db[i].module_name);
			return -1;
		}
		remote_variables[i].variable_idx = oesr_get_variable_idx(ctx, (char*) remote_params_db[i].module_name,
				(char*) remote_params_db[i].variable_name);
		if (remote_variables[i].variable_idx == -1) {
			modinfo_msg("Variable %s not found in module %s. Creating...\n",remote_params_db[i].variable_name,
					remote_params_db[i].module_name);

			/* create a new variable if not defined in .app */
			var_t new_var = oesr_var_param_create_remote(ctx,remote_variables[i].module_idx,
					(char*) remote_params_db[i].variable_name,remote_params_db[i].size);
			if (!new_var) {
				moderror_msg("Error creating remote variable %s in module %s.\n",remote_params_db[i].variable_name,
						remote_params_db[i].module_name);
				return -1;
			}
			remote_variables[i].variable_idx = new_var->id-1;
		}
		modinfo_msg("Init remote parameter %s:%s (%d,%d)\n",remote_params_db[i].module_name,
				remote_params_db[i].variable_name,remote_variables[i].module_idx,
				remote_variables[i].variable_idx);
		i++;
	}
	if (i==MAX_VARIABLES) {
		modinfo_msg("Caution: Only %d variables where parsed. "
				"Increase MAX_VARIABLES in oesr/include/ctrl_skeleton.h",i);
	}
	return i;
}

int scan_remote_itf(void *ctx, int nof_variables) {
	int i,j;
	for (i=0;i<nof_variables;i++) {
		if (!remote_variables[i].module_idx) {
			return 0;
		}
		for (j=0;j<MAX_OUTPUTS;j++) {
			if (outputs[j].module_idx == remote_variables[i].module_idx) {
				remote_variables[i].addr = &outputs[j];
				break;
			} else if (!outputs[j].module_idx) {
				remote_variables[i].addr = &outputs[j];
				outputs[j].module_idx = remote_variables[i].module_idx;
				break;
			}
		}
		if (j==MAX_OUTPUTS) {
			moderror_msg("Not enough output interfaces. Increase MAX_OUTPUTS (%d) "
					"in oesr/src/ctrl_skeleton.c\n",j);
			return -1;
		}
	}
	for (j=0;j<MAX_OUTPUTS;j++) {
		if (!outputs[j].module_idx)
			break;
	}
	return j;
}

int init_local_variables(void *ctx) {
	int i;

	i=0;
	while(local_params_db[i].name && i<MAX_VARIABLES) {
		local_variables[i] = oesr_var_param_get(ctx,(char*) local_params_db[i].name);
		if (!local_variables[i]) {
			moderror_msg("Parameter %s not found\n",local_params_db[i].name);
			return -1;
		}
		i++;
	}
	if (i==MAX_VARIABLES) {
		modinfo_msg("Caution: Only %d variables where parsed. "
				"Increase MAX_VARIABLES in oesr/include/ctrl_skeleton.h",i);
	}
	return i;
}

int close_local_variables(void *ctx, int nof_vars) {
	int i;
	for (i=0;i<nof_vars;i++) {
		if (local_variables[i]) {
			if (oesr_var_close(ctx, local_variables[i])) {
				oesr_perror("oesr_var_close\n");
			}
		}
	}
	return 0;
}

int init_remote_itf(void *ctx, int nof_itf) {
	int i,j;
	int port,delay;
	char tmp[64];

	for (i=0;i<nof_itf;i++) {
		port = outputs[i].module_idx-oesr_module_id(ctx)+nof_output_data_itf;

		/* check if a parameter sets a different delay */
		for (j=0;j<nof_remote_variables;j++) {
			if (remote_variables[j].module_idx == outputs[i].module_idx) {
				break;
			}
		}
		if (j < nof_remote_variables) {
			snprintf(tmp,64,"delay_%s",remote_params_db[j].module_name);
			if (!param_get_int_name(tmp,&delay)) {
				modinfo_msg("Setting a delay of %d slots to port %d, module %s\n",delay,port,
					remote_params_db[j].module_name);
				if (oesr_itf_delay_set(ctx,port,ITF_WRITE,delay)) {
					moderror_msg("Setting delay to port %d\n",port);
					return -1;
				}
			}
		}

		/* now create the variable */
		outputs[i].itf = oesr_itf_create(ctx, port,
				ITF_WRITE, sizeof(struct ctrl_in_pkt));
		if (outputs[i].itf == NULL) {
			if (oesr_error_code(ctx) == OESR_ERROR_NOTFOUND) {
				modinfo_msg("Caution output port %d not connected,\n",i);
			} else {
				moderror_msg("Error creating output port %d\n",i);
				oesr_perror("oesr_itf_create\n");
				return -1;
			}
		}
	}

	return 0;
}

int close_remote_itf(void *ctx, int nof_itf) {
	int i;
	if (ctrl_in) {
		if (oesr_itf_close(ctrl_in)) {
			oesr_perror("oesr_itf_close");
		}
	}
	for (i=0;i<nof_itf;i++) {
		if (oesr_itf_close(outputs[i].itf)) {
			oesr_perror("oesr_itf_close");
		}
	}
	return 0;
}


int Init(void *_ctx) {
	ctx = _ctx;

	switch(ctrl_init()) {
	case -1:
		return -1;
	case 0:
		return 0;
	}

	init_memory();

	ctrl_in = NULL;
	if (!init_ctrl_input(ctx)) {
		return 0;
	}

	nof_output_data_itf = 0;
	param_get_int_name("nof_output_data_itf",&nof_output_data_itf);

	nof_remote_variables = init_remote_variables(ctx);
	if (nof_remote_variables == -1) {
		return -1;
	}

	nof_remote_itf = scan_remote_itf(ctx,nof_remote_variables);
	if (nof_remote_itf == -1) {
		return -1;
	}

	nof_local_variables = init_local_variables(ctx);
	if (nof_local_variables == -1) {
		return -1;
	}

	if (init_remote_itf(ctx,nof_remote_itf)) {
		return -1;
	}

	return 1;
}


int local_parameters_update(void *ctx) {
	int i;

	for (i=0;i<nof_local_variables;i++) {
		if (oesr_var_param_get_value(ctx, (var_t) local_variables[i],
				local_params_db[i].value, local_params_db[i].size) == -1) {
			oesr_perror("oesr_var_param_value\n");
			return -1;
		}
	}
	return 0;
}

int process_ctrl_packet(void *ctx, struct ctrl_in_pkt *packet) {
	if (oesr_var_param_set_value_idx(ctx,packet->pm_idx,packet->value,
			packet->size) == -1) {
		oesr_perror("Error setting control parameter\n");
		return -1;
	}

	return 0;
}

int process_ctrl_packets(void *ctx) {
	int n, nof_packets;
	int i;

	do {
		n = oesr_itf_read(ctrl_in, ctrl_in_buffer,
				sizeof(struct ctrl_in_pkt)*MAX_INPUT_PACKETS,oesr_tstamp(ctx));
		if (n == -1) {
			oesr_perror("oesr_itf_read");
			return -1;
		} else if (n>0) {
			if (n % sizeof(struct ctrl_in_pkt)) {
				moderror_msg("Received %d bytes but packet size is %d\n", n,
						sizeof(struct ctrl_in_pkt));
			}
			nof_packets = n / sizeof(struct ctrl_in_pkt);
			for (i=0;i<nof_packets;i++) {
				if (process_ctrl_packet(ctx, &ctrl_in_buffer[i])) {
					moderror("Error processing control packet\n");
					return -1;
				}
			}
		}
	} while(n>0);
	return 0;
}


int Run(void *_ctx) {
	int tslot;
	ctx = _ctx;

	tslot = oesr_tstamp(ctx);
	if (ctrl_in) {
		if (process_ctrl_packets(ctx)) {
			return -1;
		}
	}

	if (local_parameters_update(ctx)) {
		moderror("Uploading control parameters\n");
		return -1;
	}

	if (ctrl_work(tslot)) {
		return -1;
	}

	if (ctrl_send_always) {
		if (remote_parameters_sendall(ctx)) {
			return -1;
		}
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int Stop(void *_ctx) {
	ctx = _ctx;
	close_remote_itf(ctx, nof_remote_itf);
	return 0;
}


int param_get(pmid_t id, void *ptr, int max_size, param_type_t *type) {
	if (type) {
		*type = (param_type_t) oesr_var_param_type(ctx,(var_t) id);
	}
	int n = oesr_var_param_get_value(ctx, (var_t) id, ptr, max_size);
	return n;
}

pmid_t param_id(char *name) {
	return (pmid_t) oesr_var_param_get(ctx,name);
}


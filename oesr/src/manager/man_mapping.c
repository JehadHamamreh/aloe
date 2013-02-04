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

#include "defs.h"
#include "man_mapping.h"
#include "man_platform.h"
#include "nod_waveform.h"
#include "mempool.h"

#include "mapper.h"

struct platform_resources plat;
struct waveform_resources wave;
struct preprocessing preproc;
struct mapping_algorithm malg;
struct cost_function costf;
struct mapping_result result;
int *join_function, *joined_function, *joined_function_inv;
float *tmp_c, *tmp_stages;
float **tmp_b;

static int mapping_alloc(mapping_t *m, int nof_modules, int nof_processors) {
	int i;

	mdebug("addr=0x%x, nof_modules=%d\n",m,nof_modules);
	if (m->p_res) return -1;
	m->p_res = (int*) pool_alloc(nof_modules,sizeof(int));
	if (!m->p_res) return -1;
	memset(m->modules_x_node,0,sizeof(int)*MAX(nodes));

    join_function = calloc(1, sizeof (int) * nof_modules);
    joined_function = calloc(1, sizeof (int) * nof_modules);
    joined_function_inv = calloc(1, sizeof (int) * nof_modules);
    tmp_stages = calloc(1, sizeof (int) * nof_modules);
	tmp_c = calloc(1, sizeof (float) * nof_modules);
    wave.c = calloc(1, sizeof (float) * nof_modules);
    wave.force = calloc(1, sizeof (int) * nof_modules);
    wave.b = calloc(1, sizeof (float*) * nof_modules);
    for (i = 0; i < nof_modules; i++) {
        wave.b[i] = calloc(1, sizeof (float) * nof_modules);
    }
    tmp_b = calloc(1, sizeof (float*) * nof_modules);
    for (i = 0; i < nof_modules; i++) {
    	tmp_b[i] = calloc(1, sizeof (float) * nof_modules);
    }
    plat.C = calloc(1, sizeof (int) * nof_processors);
    plat.B = calloc(1, sizeof (float*) * nof_processors);
    for (i = 0; i < nof_processors; i++) {
        plat.B[i] = calloc(1, sizeof (float) * nof_processors);
    }
    result.P_m = m->p_res;


	return 0;
}

static int mapping_free(mapping_t *m, int nof_modules, int nof_processors) {
	mdebug("addr=0x%x\n",m);
	int i;

	free(join_function);
	free(joined_function);
	free(joined_function_inv);
	free(tmp_stages);
	free(tmp_c);
	free(wave.c);
	free(wave.force);
	for (i=0;i<nof_modules;i++) {
		free(wave.b[i]);
	}
	free(wave.b);
	for (i=0;i<nof_modules;i++) {
		free(tmp_b[i]);
	}
	free(tmp_b);
	free(plat.C);
	for (i=0;i<nof_processors;i++) {
		free(plat.B[i]);
	}
	free(plat.B);
	if (m->p_res) {
		if (pool_free(m->p_res)) {
			return -1;
		}
		m->p_res = NULL;
	}
	return 0;
}

/** USES oesr_man_ERROR to describe any mapping error */
int setup_algorithm(mapping_t *m) {
	malg.type = tw;
	malg.w = 4;
	costf.q = 0.5;
	costf.mhop = 0;
    preproc.ord = no_ord;
    plat.arch = fd;

	return 0;
}

void generate_model_join_function(waveform_t *waveform) {
	int i,j;
	stage_config_t *s = &waveform->stages;
	int M = waveform->nof_modules;
	int min_i;

	/* initialize */
	for (i=0;i<M;i++) {
		join_function[i] = i;
	}

	for (i=0;i<s->x_len;i++) {
		min_i = s->newmodules_matrix[i][0];
		for (j=0;j<s->y_len[i];j++) {
			join_function[s->newmodules_matrix[i][j]] = min_i;
		}
	}

	mapdebug("join_function: ",0);
	for (i=0;i<M;i++) {
		mapdebug("%d,",join_function[i]);
	}
	mapdebug("\n",0);
}

void generate_model_platform(man_platform_t *platform) {
	int i,j;

	plat.nof_processors = platform->nof_processors;

	for (i=0;i<platform->nof_processors;i++) {
		plat.C[i] = platform->ts_length_us;
	}

	for (i=0;i<platform->nof_processors;i++) {
		for (j=0;j<platform->nof_processors;j++) {
			plat.B[i][j] = 1000000;
		}
	}
}

void generate_model_c_vector(waveform_t *waveform, int multiplicity) {
	int i,j,k;
	int M = waveform->nof_modules;

	memset(tmp_c,0,sizeof(float)*M);

	for (i=0;i<M;i++) {
		tmp_c[join_function[i]] += waveform->modules[i].c_mopts[0]*multiplicity;
		wave.force[i] = -1;
	}
	j=0;
	for (i=0;i<M;i++) {
		if (tmp_c[i]) {
			wave.c[j] = tmp_c[i];
			joined_function[j] = i;
			j++;
		}
	}
	for (k=0;k<j;k++) {
		for (i=0;i<M;i++) {
			if (joined_function[k] == join_function[i]) {
				joined_function_inv[i] = k;
			}
		}
	}
	mapdebug("inv:",0);
	for (i=0;i<M;i++) {
		mapdebug("%d,",joined_function_inv[i]);
	}
	mapdebug("\n",0);
	wave.nof_tasks = j;

	mapdebug("c_res=",0);
	for (i=0;i<j;i++) {
		mapdebug("%g,",wave.c[i]);
	}
	mapdebug("\n",0);
}

void generate_model_b_matrix(waveform_t *waveform, int multiplicity) {
	int i,j,k;
	int M = waveform->nof_modules;

	for (i=0;i<M;i++) {
		memset(tmp_b[i],0,sizeof(float)*M);
	}
	for (i = 0; i < M; i++) {
        for (j = 0; j < waveform->modules[i].nof_outputs; j++) {
			for (k = 0; k < M; k++) {
				if (waveform->modules[i].outputs[j].remote_module_id ==
						waveform->modules[k].id) {
					tmp_b[join_function[i]][join_function[k]] +=
							(float) waveform->modules[i].outputs[j].total_mbpts
							*multiplicity/1000;
				}
			}
        }
    }

	for (i=0;i<wave.nof_tasks;i++) {
		for (j=0;j<wave.nof_tasks;j++) {
			wave.b[i][j] = tmp_b[joined_function[i]][joined_function[j]];
		}
	}
	for (i=0;i<wave.nof_tasks;i++) {
		mapdebug("b_%d=",i);
		for (j=0;j<wave.nof_tasks;j++) {
			mapdebug("%g,",wave.b[i][j]);
		}
		mapdebug("\n",0);
	}
}

void generate_model_stages(waveform_t *waveform) {
	int i,j,k;
	int M = wave.nof_tasks;

	/* compute stages */
	for (i=0;i<M;i++) {
		tmp_stages[i] = 1;
	}

	for (i=0;i<M-1;i++) {
		for (j=i+1;j<M;j++) {
			if (wave.b[i][j] > 0) {
				if (tmp_stages[j] <= tmp_stages[i]) {
					tmp_stages[j] = tmp_stages[i]+1;
				}
			}
		}
	}

	for (i=0;i<waveform->nof_modules;i++) {
		waveform->modules[i].stage = tmp_stages[joined_function_inv[i]];
	}

	mapdebug("stages=",0);
	for (i=0;i<waveform->nof_modules;i++) {
		mapdebug("%d,",waveform->modules[i].stage);
	}
	mapdebug("\n",0);

	for (i = 0; i < waveform->nof_modules; i++) {
        for (j = 0; j < waveform->modules[i].nof_outputs; j++) {
			for (k = 0; k < waveform->nof_modules; k++) {
				if (waveform->modules[i].outputs[j].remote_module_id ==
						waveform->modules[k].id) {
					if (waveform->modules[i].stage == waveform->modules[k].stage) {
						waveform->modules[i].outputs[j].delay = 0;
					} else {
						waveform->modules[i].outputs[j].delay = 1;
					}
				}
			}
        }
	}

}

/** USES oesr_man_ERROR to describe any mapping error */
int generate_model(mapping_t *m, waveform_t *waveform, man_platform_t *platform) {

	int multiplicity;

	if (waveform->granularity_us) {
		if (platform->ts_length_us % waveform->granularity_us) {
			aerror_msg("Timeslot length must be multiple of waveform granularity (%d us)\n",
					waveform->granularity_us);
			return -1;
		}
		multiplicity = platform->ts_length_us/waveform->granularity_us;
	} else {
		multiplicity = 1;
	}

	generate_model_platform(platform);

	generate_model_join_function(waveform);

	generate_model_c_vector(waveform, multiplicity);

	generate_model_b_matrix(waveform, multiplicity);

	generate_model_stages(waveform);

	return 0;
}

/** USES oesr_man_ERROR to describe any mapping error */
int call_algorithm(mapping_t *m, waveform_t *waveform, man_platform_t *platform) {

    m->cost = mapper(&preproc, &malg, &costf, &plat, &wave, &result);
	if (m->cost == infinite) {
		printf("Error loading waveform. Not enough resources\n");
		return -1;
	} else {
		return 0;
	}
	return 0;
}

/** Maps waveform to the platform. The platform is obtained using the man_platform_get_context()
 * function.
 * The mapping result is saved to each module in the module_t.processor_idx and module_t.exec_position
 * fields. module_t.node is pointed to the platform node object of the allocated processor.
 * The number of nodes allocated to each node is saved in the mapping_t.modules_x_node array.
 * \returns 0 if the mapping was feasible, -1 otherwise. The error description is saved in the
 * oesr_man_error class.
 */
int mapping_map(mapping_t *m, waveform_t *waveform) {
	/**@TODO: Use the oesr_man_error class for error messages */

	mdebug("waveform_name=%s, nof_modules=%d\n",waveform->name, waveform->nof_modules);
	int i;
	int ret=-1;
	man_platform_t *platform = man_platform_get_context();
	man_node_t *node;
	if (!platform) {
		aerror("oesr_man not initialized\n");
		return -1;
	}
	if (mapping_alloc(m,waveform->nof_modules,platform->nof_processors)) {
		return -1;
	}
	if (setup_algorithm(m)) {
		goto free;
	}
	if (generate_model(m, waveform, platform)) {
		goto free;
	}
	if (call_algorithm(m, waveform, platform)) {
		goto free;
	}
	memset(m->modules_x_node,0,sizeof(int)*MAX(nodes));
	for (i=0;i<waveform->nof_modules;i++) {
		if (m->p_res[joined_function_inv[i]] >= platform->nof_processors) {
			aerror_msg("Module %d mapped to processor %d, but platform has %d processors only\n",
					joined_function_inv[i],m->p_res[joined_function_inv[i]]+1,platform->nof_processors);
			goto free;
		}
		man_processor_t *p = (man_processor_t*) platform->processors[m->p_res[joined_function_inv[i]]];
		waveform->modules[i].node = p->node;
		waveform->modules[i].processor_idx = p->idx_in_node;
		waveform->modules[i].exec_position = i;/*waveform->nof_modules-i-1;*/
		node = p->node;
		m->modules_x_node[node->id]++;
	}
	ret = 0;
free:
	mapping_free(m,waveform->nof_modules,platform->nof_processors);
	return ret;
}

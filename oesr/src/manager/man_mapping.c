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


static int mapping_alloc(mapping_t *m, int nof_modules, int nof_processors) {
	int i;

	mdebug("addr=0x%x, nof_modules=%d\n",m,nof_modules);
	if (m->p_res) return -1;
	m->p_res = (int*) pool_alloc(nof_modules,sizeof(int));
	if (!m->p_res) return -1;
	memset(m->modules_x_node,0,sizeof(int)*MAX(nodes));

    wave.c = calloc(1, sizeof (float) * nof_modules);
    wave.force = calloc(1, sizeof (int) * nof_modules);
    wave.b = calloc(1, sizeof (float*) * nof_modules);
    for (i = 0; i < nof_modules; i++) {
        wave.b[i] = calloc(1, sizeof (float) * nof_modules);
    }
    plat.C = calloc(1, sizeof (int) * nof_processors);
    plat.B = calloc(1, sizeof (float*) * nof_processors);
    for (i = 0; i < nof_processors; i++) {
        plat.B[i] = calloc(1, sizeof (float) * nof_processors);
    }
    result.P_m = m->p_res;


	return 0;
}

static int mapping_free(mapping_t *m) {
	mdebug("addr=0x%x\n",m);
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

/** USES oesr_man_ERROR to describe any mapping error */
int generate_model(mapping_t *m, waveform_t *waveform, man_platform_t *platform) {
	int i,j,k;

	plat.nof_processors = platform->nof_processors;

	for (i=0;i<platform->nof_processors;i++) {
		plat.C[i] = platform->ts_length_us;
	}

	for (i=0;i<platform->nof_processors;i++) {
		for (j=0;j<platform->nof_processors;j++) {
			plat.B[i][j] = 1000000;
		}
	}

	wave.nof_tasks = waveform->nof_modules;
	for (i=0;i<waveform->nof_modules;i++) {
		wave.c[i] = waveform->modules[i].c_mopts[0];
		wave.force[i] = -1;
	}

	for (i = 0; i < waveform->nof_modules; i++) {
        for (j = 0; j < waveform->modules[i].nof_outputs; j++) {
			for (k = 0; k < waveform->nof_modules; k++) {
				if (waveform->modules[i].outputs[j].remote_module_id ==
						waveform->modules[k].id) {
					wave.b[i][k] += (float) waveform->modules[i].outputs[j].total_mbpts/1000;
				}
			}
        }
    }

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

/** \brief Maps waveform to the platform. The platform is obtained using the man_platform_get_context()
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
		if (m->p_res[i] >= platform->nof_processors) {
			aerror_msg("Module %d mapped to processor %d, but platform has %d processors only\n",
					i,m->p_res[i]+1,platform->nof_processors);
			goto free;
		}
		man_processor_t *p = (man_processor_t*) platform->processors[m->p_res[i]];
		waveform->modules[i].node = p->node;
		waveform->modules[i].processor_idx = p->idx_in_node;
		waveform->modules[i].exec_position = waveform->nof_modules-i-1;
		node = p->node,
		m->modules_x_node[node->id]++;
	}
	ret = 0;
free:
	mapping_free(m);
	return ret;
}

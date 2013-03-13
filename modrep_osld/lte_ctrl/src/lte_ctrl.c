
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oesr.h>
#include <ctrl_skeleton.h>
#include <params.h>
#include "lte_lib/grid/base.h"

#include "lte_ctrl.h"

struct lte_cell_config cell;
struct lte_grid_config grid_tx,grid_rx;

int nof_symb_slot;

int x_mode;

int mode_is_tx() {
	return (!x_mode || x_mode==2);
}
int mode_is_rx() {
	return (x_mode==1 || x_mode==2);
}

int ctrl_init() {
	if (param_get_int_name("mode",&x_mode)) {
		printf("mode undefined\n");
		return -1;
	}
	grid_tx.verbose = 0;
	grid_tx.nof_pdcch = 1;
	grid_rx.subframe_idx = 1;
	grid_rx.verbose = 0;
	grid_rx.nof_pdcch = 0;
	grid_rx.subframe_idx = 0;
	return 0;
}

int _get_param(char *name, int *address, char *mode) {
	char tmp[64];
	snprintf(tmp,64,"%s_%s",name,mode);
	if (param_get_int_name(tmp,address)) {
		printf("%s undefined\n",tmp);
		return -1;
	}

	return 0;
}


int ctrl_work_(int tslot, struct lte_grid_config *grid,
		struct remote_parameters *params, char *mode) {
	int i;

	if (_get_param("cfi",&grid->cfi,mode)) {
		return -1;
	}

	if (lte_grid_init(grid)) {
		printf("Error initiating grid\n");
		return -1;
	}
	params->cfi = grid->cfi;

	params->tbs = lte_get_tbs(local_params.mcs,local_params.nrb);
	params->cbs = lte_get_cbits(local_params.mcs,local_params.nrb);
	params->modulation = lte_get_modulation_format(local_params.mcs);

	/* allocate all prb to one user */
	params->bits_x_slot = lte_pdsch_get_re(0,grid->subframe_idx,grid)*params->modulation;

	params->tslot_idx = grid->subframe_idx;

	if (grid->verbose) {
		printf("mode=%s cfi=%d sf=%d, tbs=%d, cbs=%d, mod=%d, bits=%d\n",
				mode, params->cfi, grid->subframe_idx,params->tbs,params->cbs,
				params->modulation,params->bits_x_slot);
	}
	if (grid->cfi != -1) {
		grid->subframe_idx++;
		if (grid->subframe_idx == NOF_SUBFRAMES_X_FRAME) {
			grid->subframe_idx = 0;
		}
	}
	return 0;
}

int ctrl_work(int tslot) {
	int i;

	if (mode_is_tx()) {
		ctrl_work_(tslot,&grid_tx,&tx_params, "tx");
	}
	if (mode_is_rx()) {
		ctrl_work_(tslot,&grid_rx,&rx_params, "rx");
		if (grid_rx.cfi > 0) {
			rx_params.nof_pdsch = 1;
			rx_params.pdsch_mask[0] = grid_tx.pdsch[0].rbg_mask;
		}
	}
	return 0;
}

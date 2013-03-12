
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oesr.h>
#include <ctrl_skeleton.h>
#include "lte_lib/grid/base.h"

#include "lte_ctrl_tx.h"

struct lte_cell_config cell;
struct lte_grid_config grid;

static int subframe_idx = 0;

int nof_symb_slot;

int ctrl_init() {
	int nof_subcarriers,nof_ofdm_symb_x_frame;

	grid.cfi = 1;
	grid.nof_ports = 1;
	grid.nof_prb = 6;
	grid.nof_osymb_x_subf = 14;
	grid.nof_pdcch = 1;
	grid.nof_pdsch = 1;
	grid.pdsch[0].rbg_mask=0xFFFF;
	cell.cell_id = 0;

	if (lte_grid_init(&grid)) {
		printf("Error initiating grid\n");
		return -1;
	}
	printf("Grid INIT OK\n");

	return 0;
}

int ctrl_work(int tslot) {
	int i;

	remote_params.cfi = grid.cfi;

	remote_params.tbs = lte_get_tbs(local_params.mcs,local_params.nrb);
	remote_params.cbs = lte_get_cbits(local_params.mcs,local_params.nrb);
	remote_params.modulation = lte_get_modulation_format(local_params.mcs);

	/* allocate all prb to one user */
	remote_params.bits_x_slot = lte_pdsch_get_re(0,subframe_idx,&grid)*remote_params.modulation;

	remote_params.tslot_idx = subframe_idx;

	if (grid.verbose) {
		printf("sf=%d, tbs=%d, cbs=%d, mod=%d, bits=%d\n",subframe_idx,remote_params.tbs,remote_params.cbs,
				remote_params.modulation,remote_params.bits_x_slot);
	}

	subframe_idx++;
	if (subframe_idx == NOF_SUBFRAMES_X_FRAME) {
		subframe_idx = 0;
	}
	return 0;
}

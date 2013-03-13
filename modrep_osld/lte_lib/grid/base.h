/*
 * Copyright (c) 2012,
 * Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
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

#include <complex.h>

/** @defgroup lte_lib lte_lib
 *
 * Library of common LTE routines and constants
 *
 * @{
 */

typedef _Complex float complex_t;
typedef float real_t;

#define LONG_CRC 16 /*<- This should be integrated in lte_grid_config */

#define MAX_RBG 						25
#define NOF_OSYMB_X_SLOT_NORMAL			14
#define NOF_OSYMB_X_SLOT_EXT			12
#define NOF_RE_X_OSYMB					12
#define MAX_PORTS						4
#define MAX_NPRB						110

#define MIN_CELLID 						0
#define MAX_CELLID						503
#define MAX_FFT_SIZE 					2048
#define NOF_SLOTS_X_FRAME				20
#define NOF_SUBFRAMES_X_FRAME			10

#undef DECLARE
#include "pbch.h"
#include "pcfich.h"
#include "pdcch.h"
#include "pdsch.h"
#include "phich.h"
#include "refsig.h"
#include "regs.h"
#include "sync.h"

/* The order here indicates the order they must be initialized */
#define CH_PBCH		0
#define CH_PSS		1
#define CH_SSS		2
#define CH_PCFICH	3
#define CH_PHICH	4
#define CH_PDCCH	5
#define CH_PDSCH	6
#define NOF_PHCH	8

#define CH_REF		9



struct lte_phch_config {
	char name[64];
	int code;
	unsigned short symbol_mask[NOF_SUBFRAMES_X_FRAME];
	int nof_re_x_sf[NOF_SUBFRAMES_X_FRAME];
};

struct lte_symbol {
	int subframe_id;
	int symbol_id;
};

struct lte_re {
	struct lte_symbol symbol_id;
	int re_id;
};


struct lte_grid_config {
	int fft_size;
	int pre_guard;
	int post_guard;
	int nof_re_symb;
	int nof_prb;
	int nof_osymb_x_subf;
	int nof_ports;
	int cfi;
	int cell_id;
	int phich_duration;
	float phich_ngfactor;
	int phich_ngroups;
	int nof_control_symbols;
	int nof_pdcch;
	int nof_pdsch;
	int debug;
	int verbose;
	int subframe_idx;
	struct lte_phch_config refsig_cfg[MAX_PORTS];
	struct lte_phch_config phch[NOF_PHCH];
	struct lte_pdcch pdcch[MAX_PDCCH];
	struct lte_pdsch pdsch[MAX_PDSCH];
	struct lte_grid_regs control;
};

struct lte_cell_config {
	int cell_id;
	int cell_gr;
	int cell_sec;
	int q;
};

int lte_get_cbits(int mcs, int nrb);
int lte_get_tbs(int mcs, int nrb);
int lte_get_modulation_format(int mcs);

int lte_symbol_has_ch(struct lte_symbol *symbol, struct lte_phch_config *ch);

void lte_ch_put_ref(complex_t *input, complex_t *output,int nof_prb, int offset, int ref_interval);
void lte_ch_get_ref(complex_t *input, complex_t *output,int nof_prb, int offset, int ref_interval);
void lte_ch_cp_noref(complex_t *input, complex_t *output,int nof_prb);

int lte_ch_get_re(int ch_id, int ch_type, int subframe_idx, struct lte_grid_config *config);

int lte_ch_put_sf(complex_t *input, complex_t *out_symbol, int ch_type, int channel_idx,
		int subframe_id, struct lte_grid_config *config);
int lte_ch_put_symbol(complex_t *input, complex_t *out_symbol, int ch_type, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config);

int lte_ch_get_sf(complex_t *in_symbol, complex_t *output, int ch_type, int channel_idx,
		int subframe_id, struct lte_grid_config *config);
int lte_ch_get_symbol(complex_t *in_symbol, complex_t *output, int ch_type, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config);

void lte_set_guard_symbol(complex_t *output, struct lte_grid_config *config);
void lte_set_guard_sf(complex_t *output, struct lte_grid_config *config);

int lte_get_ns(struct lte_symbol *location, struct lte_grid_config *config);
int lte_grid_init(struct lte_grid_config *config);
int lte_grid_init_params(struct lte_grid_config *config);

#define DECLARE
#include "pbch.h"
#include "pcfich.h"
#include "pdcch.h"
#include "pdsch.h"
#include "phich.h"
#include "refsig.h"
#include "regs.h"
#include "sync.h"
#undef DECLARE

/*@} */

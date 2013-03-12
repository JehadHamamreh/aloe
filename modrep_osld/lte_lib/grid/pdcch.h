
#ifndef DECLARE

#define NOF_REGS_X_CCE		9
#define MAX_PDCCH			20
#define MAX_CCE				8

struct lte_pdcch_cce {
	struct lte_reg *regs[NOF_REGS_X_CCE];
};

struct lte_pdcch {
	int id;
	int format;
	int nof_cce;
	int nof_regs;
	int nof_re;
	int nof_bits;
	struct lte_pdcch_cce cce[MAX_CCE];
};




#else
int lte_pdcch_init(struct lte_phch_config *ch, struct lte_grid_config *config);
int lte_grid_init_reg_pdcch(struct lte_pdcch *ch, struct lte_grid_config *config);
int lte_pdcch_get_bits(int ch_idx, struct lte_grid_config *config);
int lte_pdcch_get_re(int ch_idx, struct lte_grid_config *config);
int lte_pdcch_put(complex_t *pdcch, complex_t *output, int channel_idx,
		struct lte_grid_config *config);
int lte_pdcch_get(complex_t *input, complex_t *pdcch, int channel_idx,
		struct lte_grid_config *config);
#endif

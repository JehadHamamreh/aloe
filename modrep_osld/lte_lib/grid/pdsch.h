#ifndef DECLARE

#define MAX_PDSCH	10
#define MAX_RBG		25
#define MAX_RBG_SIZE	4
#define RBG_SZ(a)	(a<10?1:(a<26?2:(a<63?3:4)))



struct lte_pdsch_rbg {
	int prb_idx[MAX_RBG_SIZE];
};

/* only DL type 0 RBG-based RA is supported*/
struct lte_pdsch {
	int rbg_mask;
	int nof_re[NOF_SUBFRAMES_X_FRAME];
	int nof_rbg;
	int nof_rb;
	struct lte_pdsch_rbg rbg[MAX_RBG];
};

#else
int lte_pdsch_init(struct lte_phch_config *ch, struct lte_grid_config *config);
void lte_pdsch_setup_rbgmask(struct lte_pdsch *ch,struct lte_grid_config *config);
int lte_pdsch_fill_rbg(int *rbg_vector, struct lte_pdsch *ch, struct lte_grid_config *config);
int lte_pdsch_init_params_ch(int ch_id, struct lte_grid_config *config);
int lte_pdsch_init_sf(int subframe_id, struct lte_phch_config *ch, struct lte_grid_config *config);

int lte_pdsch_get_re(int channel_idx, int subframe_id, struct lte_grid_config *config);
int lte_pdsch_put(complex_t *pdsch, complex_t *output, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config);
int lte_pdsch_get(complex_t *input, complex_t *pdsch, int channel_idx,
		struct lte_symbol *location, struct lte_grid_config *config);
#endif

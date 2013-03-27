

#ifndef DECLARE

#define MAX_PRB		110
#define REF_L		3 /* L=0 is l=0, L=1 is l=Nsymb^DL and L=2 is l=1 */
#define REF_NS		20
#define REF_M		2*MAX_PRB


typedef struct {
	int port_id;
	complex_t signal[REF_L][REF_NS][REF_M];
	int k[REF_L][REF_NS][REF_M];
} refsignal_t;

#else
int lte_symbol_has_refsig_or_resv(struct lte_symbol *symbol, struct lte_grid_config *config);
int lte_re_has_refsig_or_resv(int k, int symbol_id, struct lte_grid_config *config);
int lte_refsig_or_resv_voffset(int symbol_id, struct lte_grid_config *config);
int lte_symbol_has_refsig(int port_id, struct lte_symbol *symbol, struct lte_grid_config *config);

int lte_refsig_init(int port_id, struct lte_phch_config *ch, struct lte_grid_config *config);

int lte_refsig_put(refsignal_t *refsignal, complex_t *output,
		struct lte_symbol *location, struct lte_grid_config *config);
int lte_refsig_get(complex_t *input, refsignal_t *refsignal,
		struct lte_symbol *location, struct lte_grid_config *config);

int lte_refsig_l(int symbol_id, struct lte_grid_config *config);
int lte_refsig_mp(int m, struct lte_grid_config *config);
void generate_cref(refsignal_t *refsignal, struct lte_grid_config *config);

#endif

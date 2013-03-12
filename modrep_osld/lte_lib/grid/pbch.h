
#ifndef DECLARE
#define PBCH_RE		4*6*NOF_RE_X_OSYMB-8*6

#else

int lte_pbch_init(struct lte_phch_config *ch, struct lte_grid_config *config);
int lte_pbch_put(complex_t *pbch, complex_t *output,
		struct lte_symbol *location, struct lte_grid_config *config);
int lte_pbch_get(complex_t *output, complex_t *pbch,
		struct lte_symbol *location, struct lte_grid_config *config);
#endif

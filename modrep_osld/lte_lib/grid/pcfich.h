
#ifndef DECLARE

#define PCFICH_REG	4
#define PCFICH_RE	PCFICH_REG*NOF_RE_X_REG

#else
int lte_pcfich_init(struct lte_phch_config *ch, struct lte_grid_config *config);
int lte_pcfich_put(complex_t *pcfich, complex_t *output,
		struct lte_grid_config *config);
int lte_pcfich_get(complex_t *input,complex_t *pcfich,
		struct lte_grid_config *config);
#endif

#ifndef DECLARE

#define PHICH_NG_MAX	2
#define PHICH_REG_MAX	((PHICH_NG_MAX*MAX_NPRB-1)/8+1)


#else
int lte_phich_init(struct lte_phch_config *ch, struct lte_grid_config *config);
int lte_phich_put(complex_t *phich, complex_t *output,
		struct lte_grid_config *config);
int lte_phich_get(complex_t *input,complex_t *phich,
		struct lte_grid_config *config);
#endif

#ifndef DECLARE

#define NOF_RE_X_REG					4

#define MAX_CTRL_SYMB	4
#define MAX_REGS_X_PRB	3


struct lte_reg {
	int k[4];
	int k0;
	int k3;
	int k_ref[2];
	int symbol;
	int id;
	int assigned;
};

struct lte_grid_regs {
	int nof_cce;
	int pdcch_nregs;
	int pcfich_nregs;
	struct lte_reg *pcfich[PCFICH_REG];
	int phich_nregs;
	struct lte_reg *phich[PHICH_REG_MAX];

	int total_nregs;
	struct lte_reg regs[MAX_NPRB][MAX_CTRL_SYMB][MAX_REGS_X_PRB];
};

#else
int lte_reg_num(int sym_id, struct lte_grid_config *config);
const char *reg_print_state(struct lte_reg *reg);
int lte_grid_init_reg(struct lte_grid_config *config, int nof_ctrl_symbols);
struct lte_reg *lte_reg_get_k(int k, int l, struct lte_grid_config *config);
int lte_reg_put(complex_t *input, complex_t *output, struct lte_reg *reg,struct lte_grid_config *config);
int lte_reg_get(complex_t *input, complex_t *output, struct lte_reg *reg,struct lte_grid_config *config);
#endif

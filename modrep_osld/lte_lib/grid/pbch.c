#include <stdio.h>
#include <string.h>
#include "base.h"

int lte_pbch_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i;
	strcpy(ch->name,"PBCH");
	ch->symbol_mask[0] = 0xF<<config->nof_osymb_x_subf/2;
	ch->nof_re_x_sf[0] = PBCH_RE;
	return 0;
}


/** 36.211 10.3 section 6.6.4
 */
int lte_pbch_put(complex_t *pbch, complex_t *output,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int k,nof_ref_x_rb;
	if (!lte_symbol_has_ch(location,&config->phch[CH_PBCH])) {
		return 0;
	}
	k=config->nof_prb*NOF_RE_X_OSYMB/2-36;
	nof_ref_x_rb = lte_symbol_has_refsig_or_resv(location,config);
	if (nof_ref_x_rb) {
		lte_ch_put_ref(pbch,&output[k],6,
				lte_refsig_or_resv_voffset(location->symbol_id,config),
				NOF_RE_X_OSYMB/nof_ref_x_rb);
		return 6*NOF_RE_X_OSYMB-nof_ref_x_rb*6;
	} else {
		lte_ch_cp_noref(pbch,&output[k],6);
		return 6*NOF_RE_X_OSYMB;
	}
}


/** 36.211 10.3 section 6.6.4
 */
int lte_pbch_get(complex_t *input, complex_t *pbch,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int k,nof_ref_x_rb;
	if (!lte_symbol_has_ch(location,&config->phch[CH_PBCH])) {
		return 0;
	}
	k=config->nof_prb*NOF_RE_X_OSYMB/2-36;
	nof_ref_x_rb = lte_symbol_has_refsig_or_resv(location,config);
	if (nof_ref_x_rb) {
		lte_ch_get_ref(&input[k],pbch,6,
				lte_refsig_or_resv_voffset(location->symbol_id,config),
				NOF_RE_X_OSYMB/nof_ref_x_rb);
		return 6*NOF_RE_X_OSYMB-nof_ref_x_rb*6;
	} else {
		lte_ch_cp_noref(&input[k],pbch,6);
		return 6*NOF_RE_X_OSYMB;
	}
}

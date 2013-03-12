#include <string.h>
#include <math.h>

#include "base.h"

int lte_pss_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i;
	strcpy(ch->name,"PSS");
	ch->symbol_mask[0] = 0x1<<(config->nof_osymb_x_subf/2-1);
	ch->symbol_mask[5] = ch->symbol_mask[0];
	ch->nof_re_x_sf[0] = PSS_RE;
	ch->nof_re_x_sf[5] = ch->nof_re_x_sf[0];
	return 0;
}

int lte_sss_init(struct lte_phch_config *ch, struct lte_grid_config *config) {
	int i;
	strcpy(ch->name,"SSS");
	ch->symbol_mask[0] = 0x1<<(config->nof_osymb_x_subf/2-2);
	ch->symbol_mask[5] = ch->symbol_mask[0];
	ch->nof_re_x_sf[0] = SSS_RE;
	ch->nof_re_x_sf[5] = ch->nof_re_x_sf[0];
	return 0;
}


/** 36.211 10.3 section 6.11.1.2
 */
int lte_pss_put(complex_t *pss, complex_t *output,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int k;
	if (!lte_symbol_has_ch(location,&config->phch[CH_PSS])) {
		return 0;
	}
	k=config->nof_prb*NOF_RE_X_OSYMB/2-31;
	memset(&output[k-5],0,5*sizeof(complex_t));
	memcpy(&output[k],pss,PSS_LEN*sizeof(complex_t));
	memset(&output[k+PSS_LEN],0,5*sizeof(complex_t));
	return PSS_LEN;
}

/** 36.211 10.3 section 6.11.2.2
 */
int lte_sss_put(real_t *sss, complex_t *output,
		struct lte_symbol *location, struct lte_grid_config *config) {
	int i,k;

	if (!lte_symbol_has_ch(location,&config->phch[CH_SSS])) {
		return 0;
	}
	k=config->nof_prb*NOF_RE_X_OSYMB/2-31;
	memset(&output[k-5],0,5*sizeof(complex_t));
	for (i=0;i<SSS_LEN;i++) {
		__real__ output[k+i] = sss[i];
		__imag__ output[k+i] = 0;
	}
	memset(&output[k+SSS_LEN],0,5*sizeof(complex_t));
	return SSS_LEN;
}

const float root_value[] = {PSSCELLID0,PSSCELLID1,PSSCELLID2};

/**
 * This function calculates the Zadoff-Chu sequence.
 * @params signal Output array.
 * @params direction 0 for tx, 1 for rx
 */
void generate_pss(complex_t *signal, int direction, struct lte_grid_config *config)
{
	int i;
	float arg;
	int root_idx;
	int phylayerID;
	int sign=direction?1:-1;

	root_idx = config->cell_id%3;

	if (config->debug) {
		for (i=0;i<PSS_LEN;i++) {
			__real__ signal[i] = 2.0;
			__imag__ signal[i] = 2.0;
		}
	} else {
		for(i=0; i<PSS_LEN/2; i++){
			arg=(float)sign*PI*root_value[root_idx]*((float)i*((float)i+1.0))/63.0;
			__real__ signal[i]=cosf(arg);
			__imag__ signal[i]=sinf(arg);
		}
		for(i=PSS_LEN/2; i<PSS_LEN; i++){
			arg=(float)sign*PI*root_value[root_idx]*(((float)i+2.0)*((float)i+1.0))/63.0;
			__real__ signal[i]=cosf(arg);
			__imag__ signal[i]=sinf(arg);
		}
	}
}


void generate_sss(real_t *signal, int direction, struct lte_grid_config *config)
{
	int i;

	if (config->debug) {
		for (i=0;i<PSS_LEN;i++) {
			signal[i] = 1.0;
		}
	}
}

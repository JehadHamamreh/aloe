#include <string.h>
#include <stdio.h>
#include <math.h>

#include "base.h"

int lte_pss_init(struct lte_phch_config *ch, struct lte_grid_config *config) {

	strcpy(ch->name,"PSS");
	ch->symbol_mask[0] = 0x1<<(config->nof_osymb_x_subf/2-1);
	ch->symbol_mask[5] = ch->symbol_mask[0];
	ch->nof_re_x_sf[0] = PSS_RE;
	ch->nof_re_x_sf[5] = ch->nof_re_x_sf[0];
	return 0;
}

int lte_sss_init(struct lte_phch_config *ch, struct lte_grid_config *config) {

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
			__real__ signal[i]=cos(arg);
			__imag__ signal[i]=sin(arg);
		}
		for(i=PSS_LEN/2; i<PSS_LEN; i++){
			arg=(float)sign*PI*root_value[root_idx]*(((float)i+2.0)*((float)i+1.0))/63.0;
			__real__ signal[i]=cos(arg);
			__imag__ signal[i]=sin(arg);
		}
	}
}



/** SECONDARY SYNCH SIGNALS*/
#define N 31
#define qp (int)floor((float)id1/30.0)
#define q  (int)floor((float)(id1+qp*(qp+1)/2)/30.0)
#define m  (int)(id1 + q*(q+1)/2)
#define z0 z[(i+m0%8)%N]
#define z1 z[(i+m1%8)%N]
#define c0 c[(i+id2)%N]
#define c1 c[(i+id2+3)%N]
#define s0 s[(i+m0)%N]
#define s1 s[(i+m1)%N]
#define NUMSSS 168

static int m0s[NUMSSS];
static int m1s[NUMSSS];
static int s[N], x[N], c[N], z[N];


/**
 *
 * This function generates the table 6.11.2.1-1 described at
 * 3GPP TS 36.211 version 10.5.0 Release 10.
 * @params
 * @params int *m0s:
 * @params int *m1s:
 */
void loadmtable (int *m0s, int *m1s){
	int id1;
	for (id1=0; id1<NUMSSS; id1++) m0s[id1] = m%N;
	for (id1=0; id1<NUMSSS; id1++) m1s[id1] = (m0s[id1]+(int)floor((float)m/31.0)+1)%N;

}


void generate_sss(real_t *signal, struct lte_grid_config *config)
{

	int i, id1 = config->cell_id/3;
	int id2 = config->cell_id%3;
	int m0;
	int m1;

	if (config->debug) {
		for (i=0;i<PSS_LEN;i++) {
			signal[i] = 1.0;
		}
		return;
	}

	loadmtable(m0s,m1s);
	m0 = m0s[id1];
	m1 = m1s[id1];

	x[0] = 0;
	x[1] = 0;
	x[2] = 0;
	x[3] = 0;
	x[4] = 1;

	for (i=0; i<26; i++) x[i+5] = (x[i+2]+x[i])%2;
	for (i=0; i<N; i++) s[i] = 1-2*x[i];

	for (i=0; i<26; i++) x[i+5] = (x[i+3]+x[i])%2;
	for (i=0; i<N; i++) c[i] = 1-2*x[i];

	for (i=0; i<26; i++) x[i+5] = (x[i+4]+x[i+2]+x[i+1]+x[i])%2;
	for (i=0; i<N; i++) z[i] = 1-2*x[i];

	for (i=0; i<N; i++){
		/** Even Resource Elements: Sub-frame 0*/
		signal[2*i] = (float)(s0*c0);
		/** Odd Resource Elements: Sub-frame 0*/
		signal[2*i+1] = (float)(s1*c1*z0);
	}
	for (i=0; i<N; i++){
		/** Even Resource Elements: Sub-frame 5*/
		signal[2*i+N*2]   = (float)(s1*c0);
		/** Odd Resource Elements: Sub-frame 5*/
		signal[2*i+1+N*2] = (float)(s0*c1*z1);
	}
}



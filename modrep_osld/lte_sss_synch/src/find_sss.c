
#include <string.h>
#include <skeleton.h>
#include "base/vector.h"
#include "base/types.h"
#include "lte_lib/grid/base.h"

#include "find_sss.h"

_Complex float sss_time[SSS_DFT_LEN];
float sss_time_real[SSS_DFT_LEN];


/* Allocate 32 complex to make it multiple of 32-byte AVX instructions alignment requirement.
 * Should use vect_malloc() to make it platform agnostic.
 */
struct fc_tables {
	complex_t z1[32][32];
	complex_t c[2][32];
	complex_t s[32][32];
};

complex_t zdelay[32],zconj[32],zprod[32];
complex_t y[2][32], z[32], tmp[32];
float tmp_real[32];

struct fc_tables fc_tables;

int decide_subframe(int m0, int m1) {
	if (m1>m0) {
		return 0;
	} else {
		return 5;
	}
}

int decide_N_id_1(int m0, int m1) {
	/** TODO:
	 *
	 */
	return 0;
}

void copy_y(complex_t *signal) {
	int i;
	for (i=0;i<N_SSS;i++) {
		y[0][i] = signal[2*i];
		y[1][i] = signal[2*i+1];
	}
}


complex_t corr_sz(complex_t *z, complex_t *s) {
	complex_t sum;
	complex_t zsprod[32];
	vec_dot_prod(z,s,zsprod,N_SSS-1);
	sum = sum_c(zsprod,N_SSS-1);

	return sum;
}
void corr_all_zs(complex_t *z, complex_t s[32][32], complex_t *output) {
	int m;
	for (m=0;m<N_SSS;m++) {
		output[m] = corr_sz(z,s[m]);
	}
}


int get_m0m1(complex_t *input, int *m0, int *m1, int correlation_threshold) {

	copy_y(input);

	vec_dot_prod(y[0],fc_tables.c[0],z,N_SSS);
	memcpy(zdelay,&z[1],(N_SSS-1)*sizeof(complex_t));
	vec_conj(z,zconj,N_SSS-1);
	vec_dot_prod(zdelay,zconj,zprod,N_SSS-1);

	corr_all_zs(zprod,fc_tables.s,tmp);
	vec_abs(tmp, tmp_real, N_SSS);
	vec_max(tmp_real, NULL, m0, N_SSS);

	modinfo_msg("m0=%g\n",tmp_real[*m0]);
	if (tmp_real[*m0] < correlation_threshold) {
		return 0;
	}

	vec_dot_prod(y[1],fc_tables.c[1],tmp,N_SSS);
	vec_dot_prod(tmp,fc_tables.z1[*m0],z,N_SSS);
	memcpy(zdelay,&z[1],(N_SSS-1)*sizeof(complex_t));
	vec_conj(z,zconj,N_SSS-1);
	vec_dot_prod(zdelay,zconj,zprod,N_SSS-1);

	corr_all_zs(zprod,fc_tables.s,tmp);
	vec_abs(tmp, tmp_real, N_SSS);
	vec_max(tmp_real, NULL, m1, N_SSS);

	if (tmp_real[*m1] < correlation_threshold) {
		return 0;
	}

	return 1;
}

void convert_tables(struct sss_tables *in) {
	int i,j;
	memset(&fc_tables,0,sizeof(struct fc_tables));
	for (i=0;i<N_SSS;i++) {
		for (j=0;j<N_SSS;j++) {
			__real__ fc_tables.z1[i][j] = (float) in->z1[i][j];
		}
	}
	for (i=0;i<N_SSS;i++) {
		for (j=0;j<N_SSS-1;j++) {
			__real__ fc_tables.s[i][j] = (float) in->s[i][j+1]*in->s[i][j];
		}
	}
	for (i=0;i<2;i++) {
		for (j=0;j<N_SSS;j++) {
			__real__ fc_tables.c[i][j] = (float) in->c[i][j];
		}
	}
}

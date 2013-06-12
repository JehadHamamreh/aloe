#include "types.h"
#include "vector.h"
#include <float.h>
#include <complex.h>
#include <stdlib.h>

#define HAVE_VOLK

#ifdef HAVE_VOLK
#include "volk/volk.h"
#endif

int sum_i(int *x, int len) {
	int i;
	int y=0;
	for (i=0;i<len;i++) {
		y+=x[i];
	}
	return y;
}

real_t sum_r(real_t *x, int len) {
#ifndef HAVE_VOLK
	int i;
	real_t y=0;
	for (i=0;i<len;i++) {
		y+=x[i];
	}
	return y;
#else
	real_t result;
	volk_32f_accumulator_s32f_a(&result,x,(unsigned int) len);
	return result;
#endif
}

complex_t sum_c(complex_t *x, int len) {
	int i;
	complex_t y=0;
	for (i=0;i<len;i++) {
		y+=x[i];
	}
	return y;
}

void vec_sum_c(complex_t *z, complex_t *x, complex_t *y, int len) {
	int i;
	for (i=0;i<len;i++) {
		z[i] = x[i]+y[i];
	}
}

void vec_mult_c_r(complex_t *x,complex_t *y, real_t h, int len) {
#ifndef HAVE_VOLK
	int i;
	for (i=0;i<len;i++) {
		y[i] = x[i]*h;
	}
#else
	complex_t hh;
	__real__ hh = h;
	__imag__ hh = 0;
	volk_32fc_s32fc_multiply_32fc_a(y,x,hh,(unsigned int) len);
#endif
}


void vec_mult_c(complex_t *x,complex_t *y, complex_t h, int len) {
	int i;
	for (i=0;i<len;i++) {
		y[i] = x[i]*h;
	}
}

void *vec_malloc(int size) {
#ifndef HAVE_VOLK
	return malloc(size);
#else
	void *ptr;
	if (posix_memalign(&ptr,64,size)) {
		return NULL;
	} else {
		return ptr;
	}
#endif
}

void vec_conj(complex_t *x, complex_t *y, int len) {
#ifndef HAVE_VOLK
	int i;
	for (i=0;i<len;i++) {
		y[i] = conjf(x[i]);
	}
#else
	volk_32fc_conjugate_32fc_a(y,x,(unsigned int) len);
#endif
}

void vec_dot_prod(complex_t *x,complex_t *y, complex_t *z, int len) {
#ifndef HAVE_VOLK
	int i;
	for (i=0;i<len;i++) {
		z[i] = x[i]*y[i];
	}
#else
	volk_32fc_x2_multiply_32fc_a(z,x,y,(unsigned int) len);
#endif
}


void vec_dot_prod_u(complex_t *x,complex_t *y, complex_t *z, int len) {
#ifndef HAVE_VOLK
	int i;
	for (i=0;i<len;i++) {
		z[i] = x[i]*y[i];
	}
#else
	volk_32fc_x2_multiply_32fc_u(z,x,y,(unsigned int) len);
#endif
}

void vec_abs(complex_t *x, real_t *abs, int len) {
#ifndef HAVE_VOLK
	int i;
	for (i=0;i<len;i++) {
		abs[i] = cabsf(x[i]);
	}
#else
	volk_32fc_magnitude_32f_a(abs,x,(unsigned int) len);

#endif

}

void vec_max(real_t *x, real_t *max, int *pos, int len) {
#ifndef HAVE_VOLK
	int i;
	real_t m=-FLT_MAX;
	int p=-1;
	for (i=0;i<len;i++) {
		if (x[i]>m) {
			m=x[i];
			p=i;
		}
	}
	if (pos) *pos=p;
	if (max) *max=m;
#else
	unsigned int target=0;
	volk_32f_index_max_16u_a(&target,x,(unsigned int) len);
	if (pos) *pos=(int) target;
	if (max) *max=x[target];
#endif
}



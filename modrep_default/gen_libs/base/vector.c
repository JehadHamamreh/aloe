#include "types.h"

int sum_i(int *x, int len) {
	int i;
	int y=0;
	for (i=0;i<len;i++) {
		y+=x[i];
	}
	return y;
}

real_t sum_r(real_t *x, int len) {
	int i;
	real_t y=0;
	for (i=0;i<len;i++) {
		y+=x[i];
	}
	return y;
}

complex_t sum_c(complex_t *x, int len) {
	int i;
	complex_t y=0;
	for (i=0;i<len;i++) {
		y+=x[i];
	}
	return y;
}


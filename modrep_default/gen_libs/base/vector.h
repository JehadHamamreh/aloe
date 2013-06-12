#include "types.h"

int sum_i(int *x, int len);
real_t sum_r(real_t *x, int len);
complex_t sum_c(complex_t *x, int len);

void *vec_malloc(int size);
void vec_sum_c(complex_t *z, complex_t *x, complex_t *y, int len);
void vec_mult_c_r(complex_t *x,complex_t *y, real_t h, int len);
void vec_mult_c(complex_t *x,complex_t *y, complex_t h, int len);
void vec_conj(complex_t *x, complex_t *y, int len);
void vec_dot_prod(complex_t *x,complex_t *y, complex_t *z, int len);
void vec_dot_prod_u(complex_t *x,complex_t *y, complex_t *z, int len);
void vec_max(real_t *x, real_t *max, int *pos, int len);
void vec_abs(complex_t *x, real_t *abs, int len);

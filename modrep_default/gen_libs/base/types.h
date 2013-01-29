#ifndef TYPES_H
#define TYPES_H

typedef float real_t;
typedef _Complex float complex_t;
typedef char bit_t;

typedef enum {
	BIT_TYPE, REAL_TYPE, COMPLEX_TYPE
} sample_t;


int type_size(sample_t type);
int type_param_2_type(int data_type, sample_t *type);

#endif

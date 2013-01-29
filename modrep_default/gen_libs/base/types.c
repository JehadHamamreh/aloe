#include "types.h"

int type_size(sample_t type) {
	switch(type) {
	case BIT_TYPE: return sizeof(bit_t);
	case REAL_TYPE: return sizeof(real_t);
	case COMPLEX_TYPE: return sizeof(complex_t);
	}
}

int type_param_2_type(int data_type, sample_t *type) {
	switch(data_type) {
	case 0: *type = BIT_TYPE; return 0;
	case 1: *type = REAL_TYPE; return 0;
	case 2: *type = COMPLEX_TYPE; return 0;
	default: return -1;
	}
}

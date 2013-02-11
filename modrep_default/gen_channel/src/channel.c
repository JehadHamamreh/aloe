#include <string.h>
#include <stdlib.h>
#include <math.h>

float rand_gauss (void) {
  float v1,v2,s;

  do {
    v1 = 2.0 * ((float) rand()/RAND_MAX) - 1;
    v2 = 2.0 * ((float) rand()/RAND_MAX) - 1;

    s = v1*v1 + v2*v2;
  } while ( s >= 1.0 );

  if (s == 0.0)
    return 0.0;
  else
    return (v1*sqrt(-2.0 * log(s) / s));
}

void gen_noise_c(_Complex float *x, float variance, int len) {
	int i;
	for (i=0;i<len;i++) {
		__real__ x[i] = rand_gauss();
		__imag__ x[i] = rand_gauss();
		x[i] *= variance;
	}
}

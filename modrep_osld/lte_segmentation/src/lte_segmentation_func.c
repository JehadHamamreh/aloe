#include <stdio.h>
#include <string.h>
#include <math.h>

#define Z  			6144
#define Bmin 		40
#define L 			24
#define NUL			0
#define CEILING(x) (int)(x) + (1 - (int)((int)((x) + 1) - (x)))

int Table5133[189];

/* @ingroup lte_segmentation
 *
 * This function performs initialization tasks for the the Transport Block Segmentation
 * in N Code Blocks (inputs blocks to turbodecoder) according LTE processing chain and
 * specifications described atETSI TS 136212 v10.6.0 (2012-07) document in section
 * 5.1.2 Code block segmentation and code block CRC attachment.
 * This function generate Table 5.1.3-3 of ETSI TS 136212 v10.6.0
 * @returns 0 on success, -1 on error
 */

int calculateTable5133(int *table) {
	int i;

	*table = 0;
	for (i = 1; i <= 60; i++) {
		table[i] = 40 + (i - 1) * 8;
	}
	for (i = 61; i <= 92; i++) {
		table[i] = 512 + (i - 60) * 16;
	}
	for (i = 93; i <= 124; i++) {
		table[i] = 1024 + (i - 92) * 32;
	}
	for (i = 93; i <= 124; i++) {
		table[i] = 1024 + (i - 92) * 32;
	}
	for (i = 125; i <= 188; i++) {
		table[i] = 2048 + (i - 124) * 64;
	}
	return (0);
}

/**
 *
 * @ingroup lte_segmentation
 *
 * This function performs the Transport Block Segmentation in N Code Blocks (inputs
 * blocks to turbodecoder) according LTE processing chain and specifications described at
 * ETSI TS 136212 v10.6.0 (2012-07) document in section 5.1.2 Code block segmentation and code
 * block CRC attachment.
 * Each code block size do not include the second CRC.
 *
 */

int get_nof_cb(int recv_bits, int *nof_short_cb, int *nof_long_cb,
		int *len_short_cb, int *len_long_cb, int *nof_filler_bits) {

	int num_cb;
	int i, Bp, Ak;

	/** Calculate Number of output code blocks*/
	if (recv_bits <= Z) {
		num_cb = 1;
		*nof_long_cb = 1;
		*nof_short_cb = 0;
		*len_short_cb = 0;
		if (recv_bits < 40) {
			*len_long_cb = 40;
			*nof_filler_bits = Bmin - recv_bits;
		} else {
			*len_long_cb = recv_bits;
			*nof_filler_bits = 0;
		}
	}

	if (recv_bits > Z) {
		num_cb = CEILING((float)recv_bits/(float)(Z - L));
		Bp = recv_bits + num_cb * L;

		/** Calculate code block sizes*/
		for (i = 1; i < 189; i++) {
			/** First Segmentation size: K+*/
			*len_long_cb = Table5133[i];
			/** Second Segmentation Size: K-*/
			*len_short_cb = Table5133[i - 1];
			if (Table5133[i] * num_cb >= Bp) {
				break;
			}
		}
		if (num_cb == 1) {
			/** C+ :Number of segments of size K+ (Kp)*/
			*nof_long_cb = 1;
			*len_short_cb = 0; /** K- */
			/** C- :Number of segments of size K- (Km)*/
			*nof_short_cb = 0;
			/** Number of Filler Bits*/
			*nof_filler_bits = Bmin - recv_bits;
		}
		if (num_cb > 1) {
			Ak = *len_long_cb - *len_short_cb;
			*nof_short_cb = (int) floor(
					((float) (num_cb * *len_long_cb) - Bp) / (float) Ak);
			*nof_long_cb = num_cb - *nof_short_cb;
			/** Number of Filler Bits*/
			*nof_filler_bits = *nof_long_cb * *len_long_cb
					+ *nof_short_cb * *len_short_cb - Bp;
		}
		*len_long_cb = *len_long_cb - 24;
		if (*len_short_cb > 0)
			*len_short_cb = *len_short_cb - 24;
	}
	return num_cb;
}


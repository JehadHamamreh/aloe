#ifndef LTE_BLKSEGFUNC_H
#define LTE_BLKSEGFUNC_H

int calculateTable5133(int *table);
int get_nof_cb(int recv_bits,\
		int *nofShortCBLKs, int *nofLongCBLKs,\
		int *shortCBLKlength, int *longCBLKlength, int *FillerBits);

#endif

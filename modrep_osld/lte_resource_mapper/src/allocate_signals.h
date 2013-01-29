int allocateDataOneUser (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, _Complex float *in, _Complex float *gridSlot);
int deallocateDataOneUser (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, _Complex float *in, _Complex float *output);
int allocateCRS (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, _Complex float *in, _Complex float *gridSlot);
int deallocateCRS (char *OFDMgrid, int fftSize,\
		int numSlot, int numsymbxSlot, _Complex float *in, _Complex float *output);
int allocatePSS (char *OFDMgrid, int fftSize, int numSlot,\
		int numOFDMsymbxSlot, _Complex float *PSSsymbols, _Complex float *gridSlot);
int allocateSSS (char *OFDMgrid, int fftSize, int numSlot,\
		int numOFDMsymbxSlot, float *SSSsymbols, _Complex float *gridSlot);


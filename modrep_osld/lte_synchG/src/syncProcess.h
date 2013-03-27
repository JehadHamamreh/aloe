

#ifndef _SYNCPROCESS_H
#define _SYNCPROCESS_H


#define MAXFFTSIZE 	4096
#define MAXINPUTLENGTH	INPUT_MAX_SAMPLES
#define CORRLENGTH	2048
//#define MAXCORRWINDOW	2048
#define CORR_THRESHOLD	7.0
//#define MAXFILTERLENGHT	128
#define MAXNOFMAX	512
#define PSS_0		0
#define PSS_1		1
#define PSS_2		2

#define SYNCH_INIT	0
#define SYNCH_1STTRACK	1
#define SYNCH_TRACK	2
#define SYNCH_LOST	3
#define SYNCH_ERROR	4

#define CP_NOR_NOFFFTsxSLOT	7	//Default number of FFTs per slot

/*#define MAXNOFPSSs	8
#define CPNOTDEF	0
#define CPNOR		1
#define CPEXT15K	2
#define CPEXT75K	3
*/
//Calculate Cyclic prefix length
#define CP_UNKNOWN			0
#define CP_NOR				1
#define CP_EXT1				2	//15kHz
#define CP_EXT2				3	//7.5kHz

#define CP_NOR_Slot1(fft_size)		((160*fft_size)/2048)
#define CP_NOR_SlotN(fft_size)		((144*fft_size)/2048)
#define CP_EXT15K(fft_size)		((512*fft_size)/2048)
#define CP_EXT75K(fft_size)		((1024*fft_size)/2048)

//calculate the number of samples in a slot including CP
#define SIZEOF_SLOT_CPNOR(fft_size)	(CP_NOR_Slot1(fft_size)+6*CP_NOR_SlotN(fft_size)+(7*fft_size))
#define SIZEOF_SLOT_CPEXT15K(fft_size)	(6*(CP_NOR_SlotN(fft_size)+fft_size))

typedef struct point2MAX{
	float MAX;			//Corr. Max. value
	float average;			//Average value
	float max2average;		//Max/average value
	float var;			//variance
	float max2var;			//Max/variance
	int pMAX;			//Point to corr. max.
	int seqNumber;			//PSS seq.:0, 1, 2
}p2MAX_t;


typedef struct subframe{
	int CPtype;
	int p2_subframe;	//point to 1st sample of subframe (including CPs)
	int nofsubframe;	//Current number of subframe in half LTE frame: 0 ..5
	int p2_OFDMPSS;		//Point to 1st sample of OFDM PSS symbol (including CP)
	int p2_OFDMSSS;		//Point to 1st sample of OFDM SSS symbol (Not including CP)
	int nofOFDMsymb;	//Current number of OFDM symb in half LTE frame: 0..NOFSYMBLTEFRAME/2
	int nofslot;		//Current number for slot in half LTE frame: 0..10
	int synchstate;		//Define the subframe detection state: INIT, TRACK
	int PSSseq;		//PSS sequence ID: 0, 1 or 2
	int phyLayerCellID;	//Detected PHY-Layer Cell ID
	int frametype;		//LTE frame type
}synchctrl_t;

int detect_CP(int fftpointer, int FFTsize, _Complex float *in);
int initframe_alignment(int numsamples, int FFTsize, _Complex float *in, _Complex float *correl,\
		_Complex float *coeff0, _Complex float *coeff1, _Complex float *coeff2,
		synchctrl_t *subframCtrl);
int frameAlignment(int numsamples, int FFTsize, _Complex float *in, _Complex float *correl,\
		_Complex float *coeff0, _Complex float *coeff1, _Complex float *coeff2,
		int CPdetected);
//int genPSStime_seq(int cellID, int FFTsize, _Complex float *PSS_time, int TxRxmode);
//int genPSStime_seqTX(int cellID, int FFTsize, fftwf_complex *PSS_time, int TxRxmode);
int vector_convolution(int numsamples, int decimate, _Complex float *in,\
			_Complex float *coeffi, int seqlength, _Complex float *corr);
//int vector_convolution(int numsamples, int decimate, _Complex float *in,\
					_Complex float *coeffi, int seqlength,\
					_Complex float *state, _Complex float *corr);
float vector_correlation(int length_in1, int decimate, _Complex float *in1,\
			_Complex float *in2, int length_in2, _Complex float *corr);
int orderMAXs(int nofMAXs, p2MAX_t *pMAXin, p2MAX_t *pMAXout);
int findMAXFinal(int numsamples, _Complex float *corr, p2MAX_t *pMAX, float threshold);
//int findfinalposMAX(int numsamples, int numblocks, int decim, int FFTsize, _Complex float *in,
//		_Complex float *coeff0, _Complex float *coeff1, _Complex float *coeff2);
void findMAX(int numsamples, int FFTsize, int decimate, _Complex float *corr, int numPSSseq, p2MAX_t *p2MAX, int *numblocks);
int synchroPSS(int numsamples, int FFTsize, _Complex float *in, _Complex float *out,\
		_Complex float *coeff0, _Complex float *coeff1, _Complex float *coeff2, p2MAX_t *pMAXout);
void setPSStime_Rx (int cellid, _Complex float *PSS, int fft_size);
int PSS_LPrealfiltering(int numsamples, _Complex float *in, _Complex float *out,
		float *LPfilter, int filterlenght);
void setPSS_LPfilter(_Complex float *LPfilter);
//void setPSS_LPfilter512(_Complex float *LPfilter);
//int PSS_LPfiltering(int numsamples, _Complex float *in, _Complex float *out,
//		_Complex float *LPfilter, int filterlenght);

#endif

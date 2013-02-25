#define DAC_NOF_CHANNELS	2
#define DAC_BUFFER_SZ		(16*1024)
#define DAC_FILENAME_LEN		128

#define DAC_SAMPLE_INT16			1
#define DAC_SAMPLE_INT32			2
#define DAC_SAMPLE_FLOAT			0


#ifdef __cplusplus

extern "C" struct dac_cfg {
	char filename[DAC_FILENAME_LEN];
	double inputFreq;
	double outputFreq;
	double inputRFFreq;
	double outputRFFreq;
	int sampleType;
	int nof_channels;
	float tx_gain;
	float rx_gain;
	float tx_bw;
	float rx_bw;
	int chain_is_tx;
	int NsamplesIn;
	int NsamplesOut;
	char dacinbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
	char dacoutbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
};

#else
#include <math.h>
#include <complex.h>

struct dac_cfg {
	char filename[DAC_FILENAME_LEN];
	double inputFreq;
	double outputFreq;
	double inputRFFreq;
	double outputRFFreq;
	int sampleType;
	int nof_channels;
	float tx_gain;
	float rx_gain;
	float tx_bw;
	float rx_bw;
	int chain_is_tx;
	int NsamplesIn;
	int NsamplesOut;
	char dacinbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
	char dacoutbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
};

#endif



#ifdef __cplusplus
extern "C" {
#endif


int uhd_open(char *args, void **handler);
int uhd_close(void *h);
int uhd_start_rx_stream(void *h);

float uhd_set_tx_srate(void *h, float freq);
float uhd_get_tx_srate(void *h);
float uhd_set_rx_srate(void *h, float freq);
float uhd_get_rx_srate(void *h);

float uhd_set_tx_gain(void *h, float gain);
float uhd_set_rx_gain(void *h, float gain);

float uhd_set_tx_freq(void *h, float freq);
float uhd_set_rx_freq(void *h, float freq);

int uhd_send(void *h, void *data, int nsamples, int blocking);
int uhd_recv(void *h, void *data, int nsamples, int blocking);


#ifdef __cplusplus
}
#endif

#include <uhd/usrp/multi_usrp.hpp>
#include <iostream>
#include <complex>
#include <cstdio>

#include "uhd_handler.hpp"
#include "uhd.h"
#include "ring_buff.h"

typedef _Complex float complex_t;

#define SAMPLE_SZ sizeof(complex_t)

#define USE_THREAD

void uhd_rx_stream(void *h);

#ifdef USE_THREAD
void *rx_thread(void *args) {
	uhd_handler* handler = static_cast<uhd_handler*>(args);
	size_t nsamples = handler->rx_stream->get_max_num_samps();
	ring_buff_err_t err;

#ifdef __XENO__
	pthread_set_mode_np(PTHREAD_WARNSW,0);
#endif

	complex_t *buff;
	uhd::rx_metadata_t md;
	int n;
	while(1) {
		err = ring_buff_reserve(handler->ring_buff, (void**)&buff, nsamples*SAMPLE_SZ);
		if(err != RING_BUFF_ERR_OK)
		{
			printf("************* ERROR reserving message **************\n");
			ring_buff_print_err(err);
			return NULL;
		}
		n=0;
		do {
			n += handler->rx_stream->recv(&buff[n], nsamples-n, md,1.0,true);
		} while(n<nsamples);
		err = ring_buff_commit(handler->ring_buff, buff, nsamples*SAMPLE_SZ);
		if(err != RING_BUFF_ERR_OK)
		{
			printf("************* ERROR committing message *************\n");
			ring_buff_print_err(err);
			return NULL;
		}
	}

}


int create_ring_buffer(uhd_handler* handler, int size) {
	ring_buff_attr_t   ring_buff_attr = {NULL, 0, 0, NULL,0};
	ring_buff_err_t err;
	void *buff = malloc(size);
	if (!buff) {
		printf("error creating buffer\n");
		return -1;
	}
	ring_buff_attr.buff = buff;
	ring_buff_attr.size = size;
	err = ring_buff_create(&ring_buff_attr, &handler->ring_buff);
	if(err != RING_BUFF_ERR_OK)
	{
		printf("************** ERROR creating ring buffer **************\n");
		ring_buff_print_err(err);
		return -1;
	}
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if (pthread_attr_setschedpolicy(&attr, SCHED_OTHER)) {
		printf("error setting sched policy\n");
	}
	/*
	struct sched_param param;
	param.sched_priority = 20;
	if (pthread_attr_setschedparam(&attr, &param)) {
		printf("error setting priority\n");
	}
	*/
	if (pthread_create(&handler->rx_thread, &attr, rx_thread, handler) != 0)
	{
		printf("************ ERROR creating provider thread *************\n");
		pthread_attr_destroy(&attr);
		return -1;
	}

	return 0;
}
#endif

int uhd_start_rx_stream(void *h) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
    uhd::stream_cmd_t cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    cmd.time_spec = handler->usrp->get_time_now();
    cmd.stream_now = true;
    handler->usrp->issue_stream_cmd(cmd);
    return 0;
}


int uhd_open(char *args, void **h) {
	uhd_handler* handler = new uhd_handler();
	std::string _args=std::string(args);
	handler->usrp= uhd::usrp::multi_usrp::make(_args);

	std::string otw, cpu;
	otw="sc16";
	cpu="fc32";

	uhd::stream_args_t stream_args(cpu, otw);
	stream_args.channels.push_back(0);
	stream_args.args["noclear"] = "1";
	handler->tx_stream = handler->usrp->get_tx_stream(stream_args);
    handler->tx_md.time_spec = handler->usrp->get_time_now();
    handler->tx_md.has_time_spec = false;

	handler->rx_stream = handler->usrp->get_rx_stream(stream_args);
	*h = handler;

	int size = 10000*handler->rx_stream->get_max_num_samps();

#ifdef USE_THREAD
	if (create_ring_buffer(handler,size)) {
		return -1;
	}
#endif

	return 0;
}

int uhd_close(void *h) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	return 0;
}

float uhd_set_tx_srate(void *h, float freq) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	handler->usrp->set_tx_rate(freq);
	return handler->usrp->get_tx_rate();
}

float uhd_get_tx_srate(void *h) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	return handler->usrp->get_tx_rate();
}

float uhd_set_rx_srate(void *h, float freq) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	handler->usrp->set_rx_rate(freq);
	return handler->usrp->get_rx_rate();
}

float uhd_get_rx_srate(void *h) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	return handler->usrp->get_tx_rate();
}

float uhd_set_tx_gain(void *h, float gain) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	handler->usrp->set_tx_gain(gain);
	return handler->usrp->get_tx_gain();
}

float uhd_set_rx_gain(void *h, float gain) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	handler->usrp->set_rx_gain(gain);
	return handler->usrp->get_rx_gain();
}

float uhd_set_tx_freq(void *h, float freq) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	handler->usrp->set_tx_freq(freq);
	return handler->usrp->get_tx_freq();
}

float uhd_set_rx_freq(void *h, float freq) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	handler->usrp->set_rx_freq(freq);
	return handler->usrp->get_rx_freq();
}


int uhd_send(void *h, void *data, int nsamples, int blocking) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	if (blocking) {
		int n=0;
		char *data_c = (char*) data;
		do {
			n+=handler->tx_stream->send(&data_c[n], nsamples-n, handler->tx_md);
		}while(n<nsamples);
		return nsamples;
	} else {
		return handler->tx_stream->send(data, nsamples, handler->tx_md,0.0);
	}
}

size_t uhd_recv_rb(uhd_handler* handler, void *data, int nbytes, int blocking) {
	void *buff;
	size_t r;
	ring_buff_err_t err;
	err = ring_buff_read(handler->ring_buff, &buff, nbytes, &r,blocking);
	if(err != RING_BUFF_ERR_OK)
	{
		printf("************** ERROR reading message ***************\n");
		ring_buff_print_err(err);
		return -1;
	}
	if (!buff || !r) {
		return 0;
	}
	memcpy(data,buff,r);
	err = ring_buff_free(handler->ring_buff, buff, r);
	if(err != RING_BUFF_ERR_OK)
	{
		printf("************** ERROR freeing message ***************\n");
		ring_buff_print_err(err);
		return -1;
	}
	return r;
}

#ifdef USE_THREAD
int uhd_recv(void *h, void *data, int nsamples, int blocking) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	size_t r=0;
	char *data_c = (char*) data;

	r = uhd_recv_rb(handler,data_c,nsamples*SAMPLE_SZ,blocking);
	if (!r && !blocking) {
		return 0;
	}
	/* buffer wrapped */
	if (r<nsamples*SAMPLE_SZ) {
		r+=uhd_recv_rb(handler,&data_c[r],nsamples*SAMPLE_SZ-r,blocking);
		if (r<nsamples*SAMPLE_SZ) {
			printf("this should not happen\n");
			return -1;
		}
	}

	return nsamples;
}
#else

int uhd_recv(void *h, void *data, int nsamples, int blocking) {
	uhd_handler* handler = static_cast<uhd_handler*>(h);
	uhd::rx_metadata_t md;
	if (blocking) {
		int n=0;
		complex_t *data_c = (complex_t*) data;
		do {
			n+=handler->rx_stream->recv(&data_c[n], nsamples-n, md);
		} while(n<nsamples);
		return nsamples;
	} else {
		return handler->rx_stream->recv(data, nsamples, md, 0.0);
	}
}
#endif

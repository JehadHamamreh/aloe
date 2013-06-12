#include <uhd/usrp/multi_usrp.hpp>
#include <pthread.h>
#include "ring_buff.h"

class uhd_handler {
public:
	uhd::usrp::multi_usrp::sptr usrp;

	uhd::rx_streamer::sptr rx_stream;
	bool rx_stream_enable;
	pthread_t rx_thread;

	uhd::tx_streamer::sptr tx_stream;
	uhd::tx_metadata_t tx_md;
	ring_buff_handle_t ring_buff;

};

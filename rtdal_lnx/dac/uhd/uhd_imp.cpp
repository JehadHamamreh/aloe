//
// Copyright 2011-2012 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <uhd/utils/thread_priority.hpp>
#include <uhd/convert.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <complex>
#include <cstdio>

#include <signal.h>
#include <semaphore.h>
#include <string.h>
#include "uhd.h"
#include "dac_cfg.h"

/***********************************************************************
 * Test result variables
 **********************************************************************/
unsigned long long num_overflows = 0;
unsigned long long num_underflows = 0;
unsigned long long num_rx_samps = 0;
unsigned long long num_tx_samps = 0;
unsigned long long num_dropped_samps = 0;
unsigned long long num_seq_errors = 0;

#define SAVE_LEN 100000
int save_period[SAVE_LEN];
int save_idx=0;

struct timeval t[3];
double mean_interval=0;
int ct=0;
struct dac_cfg *def_cfg;

/***********************************************************************
 * Benchmark RX Rate
 **********************************************************************/
void rx_thread(uhd::usrp::multi_usrp::sptr usrp, const std::string &rx_cpu,
		const std::string &rx_otw, struct dac_cfg *cfg, void (*sync)(void), long int *tslen_ptr){

    double rx_rate=0;

    //create a receive streamer
    uhd::stream_args_t stream_args(rx_cpu, rx_otw);
    stream_args.channels.push_back(0); //single-channel
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);

    //setup variables and allocate buffer
    uhd::rx_metadata_t md;

    std::vector<void *> buffs;
    buffs.push_back(cfg->dacinbuff[0]); //only 1 channel is used
    bool had_an_overflow = false;
    uhd::time_spec_t last_time;
    const double rate = usrp->get_rx_rate();

    uhd::stream_cmd_t cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    cmd.time_spec = usrp->get_time_now() + uhd::time_spec_t(0.05);
    cmd.stream_now = (buffs.size() == 1);
    usrp->issue_stream_cmd(cmd);
    while (not boost::this_thread::interruption_requested()){
    	/* Poll if sampling frequency has changed */
		if (cfg->inputFreq!=rx_rate) {
			usrp->set_rx_rate(cfg->inputFreq);
			rx_rate=cfg->inputFreq;
		}

	    double x=(double) cfg->NsamplesOut/rx_rate;
	    *tslen_ptr=(long int) 1000000*x;

        num_rx_samps += rx_stream->recv(buffs, cfg->NsamplesIn, md);

        sync();

        //handle the error codes
        switch(md.error_code){
        case uhd::rx_metadata_t::ERROR_CODE_NONE:
            if (had_an_overflow){
                had_an_overflow = false;
                num_dropped_samps += (int) (md.time_spec - last_time).get_real_secs()*rate;
            }
            break;

        case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
            had_an_overflow = true;
            last_time = md.time_spec;
            num_overflows++;
            break;

        default:
            std::cerr << "Error code: " << md.error_code << std::endl;
            std::cerr << "Unexpected error on recv, continuing..." << std::endl;
            break;
        }

    }
    usrp->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}

void time_interval(struct timeval * tdata)
{

    /*First element stores the result,
     *second element is the start time and
     *third element is the end time.
     */
    tdata[0].tv_sec = tdata[2].tv_sec - tdata[1].tv_sec;
    tdata[0].tv_usec = tdata[2].tv_usec - tdata[1].tv_usec;
    if (tdata[0].tv_usec < 0) {

        tdata[0].tv_sec--;
        tdata[0].tv_usec += 1000000;
    }
}
/***********************************************************************
 * Benchmark TX Rate
 **********************************************************************/
void tx_thread(uhd::usrp::multi_usrp::sptr usrp,
		const std::string &tx_cpu, const std::string &tx_otw, struct dac_cfg *cfg,
		void (*sync_ts)(void), long int *tslen_ptr){

    struct timeval t[3];
    double tx_rate=0;
    int nsamples,n;
    int a=0;

    def_cfg = cfg;

    //create a transmit streamer
    uhd::stream_args_t stream_args(tx_cpu, tx_otw);
    stream_args.channels.push_back(0); // only one channel is used.
    uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);

    //setup variables buffer
    uhd::tx_metadata_t md;
    md.time_spec = usrp->get_time_now() + uhd::time_spec_t(0.05);
    md.has_time_spec = false;

    std::vector<void *> buffs;
    buffs.push_back(cfg->dacoutbuff[0]); //only 1 channel is used

    while (not boost::this_thread::interruption_requested()) {
    	/* Poll if sampling frequency has changed */

	    if (cfg->outputFreq!=tx_rate) {
			std::cout << "Setting TX Frequency to " << cfg->outputFreq << std::endl;
			usrp->set_tx_rate(cfg->outputFreq);
			tx_rate = cfg->outputFreq;
			a++;
		}

	    sync_ts();
/*
		for (size_t n = 0; n < cfg->NsamplesOut; n++) {
			if (cfg->sampleType) {
				buff_s[n] = dacbuff_s[n];
			} else {
				buff_f[n] = dacbuff_f[n];
			}
		}
*/
		tx_stream->send(buffs, cfg->NsamplesOut, md);
        md.has_time_spec = false;
    }

    //send a mini EOB packet
    md.end_of_burst = true;
    tx_stream->send(buffs, 0, md);
}

void benchmark_tx_rate_async_helper(uhd::usrp::multi_usrp::sptr usrp){
	uhd::async_metadata_t async_md;

    while (not boost::this_thread::interruption_requested()){

        if (not usrp->get_device()->recv_async_msg(async_md)) continue;

        //handle the error codes
        switch(async_md.event_code){
        case uhd::async_metadata_t::EVENT_CODE_BURST_ACK:
            return;

        case uhd::async_metadata_t::EVENT_CODE_UNDERFLOW:
        case uhd::async_metadata_t::EVENT_CODE_UNDERFLOW_IN_PACKET:
            	num_underflows++;
            break;

        case uhd::async_metadata_t::EVENT_CODE_SEQ_ERROR:
        case uhd::async_metadata_t::EVENT_CODE_SEQ_ERROR_IN_BURST:
            num_seq_errors++;
            break;

        default:
            std::cerr << "Event code: " << async_md.event_code << std::endl;
            std::cerr << "Unexpected event on async recv, continuing..." << std::endl;
            break;
        }
    }
}
boost::thread_group thread_group;
uhd::usrp::multi_usrp::sptr usrp;

struct main_conf main_cfg;

void uhd_setcfg(struct main_conf *main, struct dac_cfg *cfg) {
	std::string args=std::string("");

	//create a usrp device
	usrp = uhd::usrp::multi_usrp::make(args);

	usrp->set_tx_rate(cfg->outputFreq);
	cfg->outputFreq = usrp->get_tx_rate();

	/* and save configuration */
	memcpy(&main_cfg,main,sizeof(struct main_conf));
}


/***********************************************************************
 * Main code + dispatcher
 **********************************************************************/
int uhd_init(struct dac_cfg *cfg, long int *timeSlotLength, void (*sync)(void)){

    std::string rx_otw, tx_otw;
    std::string rx_cpu, tx_cpu;
    rx_otw="sc16";
	tx_otw="sc16";

	if (cfg->sampleType) {
		rx_cpu="sc16";
		tx_cpu="sc16";
	} else {
		rx_cpu="fc32";
		tx_cpu="fc32";
	}

    //spawn the receive test thread
    if (main_cfg.chain_is_tx==0) {
    	/** Set receive chain */
		for(size_t chan = 0; chan < usrp->get_rx_num_channels(); chan++) {
			double freq = cfg->inputRFFreq;
			if (freq>0) {
				usrp->set_rx_freq(freq, chan);
			}

			//set the rf gain
			double gain = cfg->rx_gain;
			if (gain>0) {
				usrp->set_rx_gain(gain, chan);
			}

			//set the IF filter bandwidth
			double bw = cfg->rx_bw;
			if (bw>0) {
				usrp->set_rx_bandwidth(bw, chan);
			}
		}

    	double x=(double) cfg->NsamplesOut/cfg->inputFreq;
    	*timeSlotLength=(int) 1000000*x;
		thread_group.create_thread(boost::bind(&rx_thread, usrp, rx_cpu, rx_otw,cfg,sync,timeSlotLength));
    }

    //spawn the transmit test thread
	if (main_cfg.chain_is_tx==1) {

		/** Set transmit chain */
		for(size_t chan = 0; chan < usrp->get_tx_num_channels(); chan++) {
			double freq = cfg->outputRFFreq;
			if (freq>0) {
				usrp->set_tx_freq(freq, chan);
			}

			//set the rf gain
			double gain = cfg->tx_gain;
			if (gain>0.0) {
				usrp->set_tx_gain(gain, chan);
			}

			//set the IF filter bandwidth
			double bw = cfg->tx_bw;
			if (bw>0.0) {
				usrp->set_tx_bandwidth(bw, chan);
			}
			printf("\nTX Chain: Freq=%g MHz\tBW=%g MHz\tGain=%g dB\n",freq/1000000,bw/1000000,gain);
		}

		double x=(double) cfg->NsamplesOut/cfg->outputFreq;
		*timeSlotLength=(int) 1000000*x;
		thread_group.create_thread(boost::bind(&tx_thread, usrp, tx_cpu, tx_otw,cfg,sync,timeSlotLength));
	}
	return 1;
}

void uhd_close() {

    //	interrupt and join the threads
    thread_group.interrupt_all();
    thread_group.join_all();

    //print summary
    /*printf(
        "Benchmark rate summary:\n"
        "  Num received samples:    %llu\n"
        "  Num dropped samples:     %llu\n"
        "  Num overflows detected:  %llu\n"
        "  Num transmitted samples: %llu\n"
        "  Num sequence errors:     %llu\n"
        "  Num underflows detected: %llu\n"
    ,num_rx_samps , num_dropped_samps , num_overflows , num_tx_samps ,
    num_seq_errors , num_underflows);
	*/
}

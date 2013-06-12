/*
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 *
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "rtdal_dac.h"
#include "rtdal.h"
#include "defs.h"
#include "str.h"

#define cast(a,b) rtdal_dac_t* a = (rtdal_dac_t*) b


#define call(a, ...) do {switch(dac->code) {\
	case DAC_USRP: return call_usrp(a,__VA_ARGS__); \
	default: return -1;}}while(0)

int dac_open(rtdal_dac_t *dac, string args) {
	call(open,args,&dac->handler);
}

static rtdal_dac_t handlers[MAX_DAC_HANDLERS];

r_dac_t rtdal_dac_open(string name, string args) {

	int i=0;
	while(dac_names[i].name && strcmp(name,"USRP")) {
		i++;
	}
	if (!dac_names[i].name) {
		printf("Unknown DAC name %s\n",name);
		return NULL;
	}
	int j=0;
	while(j<MAX_DAC_HANDLERS && handlers[j].id) {
		j++;
	}
	if (j==MAX_DAC_HANDLERS) {
		printf("No more DAC handlers\n");
		return NULL;
	}
	handlers[j].code = dac_names[i].code;

	if (dac_open(&handlers[j],args)) {
		return NULL;
	}

	return (r_dac_t) &handlers[j];
}

int rtdal_dac_close(r_dac_t obj) {
	cast(dac,obj);
	call(close,dac->handler);
}

int rtdal_dac_start_rx_stream(r_dac_t obj) {
	cast(dac,obj);
	call(start_rx_stream,dac->handler);
}

float rtdal_dac_set_tx_srate(r_dac_t obj, float freq) {
	cast(dac,obj);
	call(set_tx_srate,dac->handler,freq);
}

float rtdal_dac_get_tx_srate(r_dac_t obj) {
	cast(dac,obj);
	call(get_tx_srate,dac->handler);
}

float rtdal_dac_set_rx_srate(r_dac_t obj, float freq) {
	cast(dac,obj);
	call(set_rx_srate,dac->handler,freq);
}

float rtdal_dac_get_rx_srate(r_dac_t obj) {
	cast(dac,obj);
	call(get_rx_srate,dac->handler);
}

float rtdal_dac_set_tx_gain(r_dac_t obj, float gain) {
	cast(dac,obj);
	call(set_tx_gain,dac->handler,gain);
}
float rtdal_dac_set_rx_gain(r_dac_t obj, float gain) {
	cast(dac,obj);
	call(set_rx_gain,dac->handler,gain);
}


float rtdal_dac_set_tx_freq(r_dac_t obj, float freq) {
	cast(dac,obj);
	call(set_tx_freq,dac->handler,freq);
}
float rtdal_dac_set_rx_freq(r_dac_t obj, float freq) {
	cast(dac,obj);
	call(set_rx_freq,dac->handler,freq);
}


int rtdal_dac_send(r_dac_t obj, void *data, int nsamples, int blocking) {
	cast(dac,obj);
	call(send,dac->handler,data,nsamples,blocking);
}
int rtdal_dac_recv(r_dac_t obj, void *data, int nsamples, int blocking) {
	cast(dac,obj);
	call(recv,dac->handler,data,nsamples,blocking);
}

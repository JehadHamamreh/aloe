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

#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "ring_buff_osal.h"

#include "rtdal.h"
#include "rtdal_error.h"
#include "rtdal_itf.h"
#include "rtdal_itfspscq.h"
#include "defs.h"
#include "str.h"

#define USE_SYSTEM_TSTAMP

typedef struct {
	int valid;
	int tstamp;
	int len;
	void *data;
}r_pkt_t;

typedef struct {
	rtdal_itf_t parent;

	int max_msg;
	int max_msg_sz;
	int read;
	int write;
	ring_buff_binary_sem_t sem_r;
	ring_buff_binary_sem_t sem_w;
	r_pkt_t *packets;
	char *data;
}rtdal_itfspscq_t;

#define cast(a,b) RTDAL_ASSERT_PARAM(a);\
			rtdal_itfspscq_t *b = (rtdal_itfspscq_t*) a;
#define cast_p(a,b) RTDAL_ASSERT_PARAM_p(a);\
			rtdal_itfspscq_t *b = (rtdal_itfspscq_t*) a;

static int spscq_id=1;

r_itf_t rtdal_itfspscq_new(int max_msg, int msg_sz, int delay, r_log_t log) {
	int i;
	rtdal_itfspscq_t *itf = malloc(sizeof(rtdal_itfspscq_t));
	if (!itf) {
		return NULL;
	}

	itf->parent.type = ITF_INT_SPSCQ;
	itf->max_msg = max_msg;
	itf->max_msg_sz = msg_sz;
	itf->parent.delay = delay;
	itf->parent.log = log;
	itf->parent.id=spscq_id++;
	itf->read = 0;
	itf->write = 0;
	posix_memalign((void**)&itf->data,64,itf->max_msg*itf->max_msg_sz);
	memset(itf->data,0,itf->max_msg*itf->max_msg_sz);
	itf->packets = calloc(itf->max_msg,sizeof(r_pkt_t));
	if (!itf->data || !itf->packets) {
		return NULL;
	}
	for (i=0;i<itf->max_msg;i++) {
		itf->packets[i].data = &itf->data[i*itf->max_msg_sz];
		itf->packets[i].valid = 0;
	}
	if (itf->parent.delay < 0) {
		ring_buff_binary_sem_create(&itf->sem_r);
		ring_buff_binary_sem_create(&itf->sem_w);
	}

	return (r_itf_t) itf;
}

int rtdal_itfspscq_reset(r_itf_t obj) {
	cast(obj,itf);
	int i;

	itf->read = 0;
	itf->write = 0;
	qdebug("resetting %d msg \n",itf->max_msg);
	for (i=0;i<itf->max_msg;i++) {
		itf->packets[i].valid = 0;
	}
	return 0;
}

int rtdal_itfspscq_remove(r_itf_t obj) {
	cast(obj,itf);
	if (itf->data) {
		free(itf->data);
		itf->data = NULL;
	}
	if (itf->packets) {
		free(itf->packets);
		itf->packets = NULL;
	}
	itf->max_msg = 0;
	itf->max_msg_sz = 0;
	itf->parent.id = 0;

	//ring_buff_binary_sem_destroy(itf->sem);

	free(itf);

	return 0;
}


inline static int spscq_is_empty(rtdal_itfspscq_t *itf, int tstamp) {
	if (itf->parent.delay >= 0 || itf->parent.delay == -2) {
		return (itf->packets[itf->read].valid == 0);
	} else {
		while (itf->packets[itf->read].valid == 0 && itf->packets[itf->read].tstamp > tstamp) {
			qdebug("wait write=%d read=%d\n",itf->write,itf->read);
			ring_buff_binary_sem_take(itf->sem_r);
		}
		return 0;
	}

}

inline static int spscq_is_full(rtdal_itfspscq_t *itf) {
	if (itf->parent.delay >= 0) {
		return (itf->packets[itf->write].valid != 0);
	} else {
		while(itf->packets[itf->write].valid != 0) {
			qdebug("wait write=%d read=%d\n",itf->write,itf->read);
			ring_buff_binary_sem_take(itf->sem_w);
		}
		return 0;
	}
}

int rtdal_itfspscq_request(r_itf_t obj, void **ptr) {
	cast(obj,itf);
	RTDAL_ASSERT_PARAM(ptr);

	*ptr = NULL;
	qdebug("write=%d read=%d\n",itf->write,itf->read);

	if (spscq_is_full(itf)) {

		qdebug("[full] id=%d write=%d, valid=%d\n",itf->parent.id,itf->write,itf->packets[itf->write].valid);
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		return 0;
	}

	qdebug("[ok] write=%d/%d\n",itf->write,itf->max_msg);
	*ptr = itf->packets[itf->write].data;


	return 1;
}

int rtdal_itfspscq_push(r_itf_t obj, void *ptr, int len, int tstamp) {
	cast(obj,itf);
	if (!len) {
		return 1;
	}
	/*
	if (spscq_is_full(itf)) {
		qdebug("[full] write=%d read=%d\n",itf->write,itf->read);
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		return 0;
	}
	*/

#ifdef USE_SYSTEM_TSTAMP
	tstamp=rtdal_time_slot();
#endif

	itf->packets[itf->write].tstamp = tstamp+itf->parent.delay;
	itf->packets[itf->write].len = len;
	itf->packets[itf->write].valid = 1;
	qdebug("write=%d/%d, len=%d, tstamp=%d, delay=%d\n",itf->write,itf->max_msg,len,tstamp,itf->parent.delay);

	itf->write += (itf->write+1 >= itf->max_msg) ? (1-itf->max_msg) : 1;

	if (itf->parent.delay < 0) {
		ring_buff_binary_sem_give(itf->sem_r);
	}

	return 1;
}

int rtdal_itfspscq_pop(r_itf_t obj, void **ptr, int *len, int tstamp) {
	cast(obj,itf);
	RTDAL_ASSERT_PARAM(ptr);
	RTDAL_ASSERT_PARAM(len);

	*ptr = NULL;
	*len = 0;

	if (spscq_is_empty(itf,tstamp)) {
		if (itf->parent.delay==-2) {
			usleep(1000);
		}
		qdebug("[empty] read=%d write=%d\n",itf->read,itf->write);
		return 0;
	}

	if (itf->parent.delay >= 0 ) {
#ifdef USE_SYSTEM_TSTAMP
		tstamp=rtdal_time_slot();
#endif
		if (itf->packets[itf->read].tstamp > tstamp) {
			qdebug("[delay] read=%d, tstamp=%d now=%d\n",itf->read,itf->packets[itf->read].tstamp,tstamp);
			return 0;
		}
/*
		if (itf->packets[itf->read].tstamp < tstamp-1) {
			qdebug("[old] read=%d, tstamp=%d now=%d\n",itf->read,itf->packets[itf->read].tstamp,tstamp);
			rtdal_itfspscq_release(obj,NULL,0);
			return 2;
		}
*/
	}

	qdebug("[ok] read=%d, tstamp=%d (now=%d)\n",itf->read,itf->packets[itf->read].tstamp,tstamp);
	*ptr = itf->packets[itf->read].data;
	*len = itf->packets[itf->read].len;

	return 1;
}

int rtdal_itfspscq_release(r_itf_t obj, void *ptr, int len) {
	cast(obj,itf);
	/*
	if (spscq_is_empty(itf)) {
		qdebug("[empty] read=%d, write=%d\n",itf->read,itf->write);
		return 0;
	}
	*/

	itf->packets[itf->read].valid = 0;
	itf->read += (itf->read+1 >= itf->max_msg) ? (1-itf->max_msg) : 1;
	qdebug("read=%d, write=%d\n",itf->read,itf->write);

	if (itf->parent.delay < 0) {
		ring_buff_binary_sem_give(itf->sem_w);
	}

	return 1;
}

int rtdal_itfspscq_set_delay(r_itf_t obj, int delay) {
	aerror("Not yet implemented");
	return -1;
}

int rtdal_itfspscq_get_delay(r_itf_t obj) {
	aerror("Not yet implemented");
	return -1;
}


int rtdal_itfspscq_set_callback(r_itf_t obj, void (*fnc)(void), int prio) {
	aerror("Not yet implemented");
	return -1;
}

int rtdal_itfspscq_set_blocking(r_itf_t obj, int block) {
	aerror("Not yet implemented");
	return -1;
}

int rtdal_itfspscq_get_blocking(r_itf_t obj) {
	aerror("Not yet implemented");
	return -1;
}


int rtdal_itfspscq_send(r_itf_t obj, void* buffer, int len, int tstamp) {
	cast(obj,itf);
	RTDAL_ASSERT_PARAM(buffer);
	RTDAL_ASSERT_PARAM(len>=0);

	int n;
	void *ptr;

	if (len > itf->max_msg_sz) {
		RTDAL_SETERROR(RTDAL_ERROR_LARGE);
		return -1;
	}

	if ((n = rtdal_itfspscq_request(obj, &ptr)) != 1) {
		return n;
	}

	memcpy(ptr, buffer, (size_t) len);

	return rtdal_itfspscq_push(obj,ptr,len,tstamp);
}

int rtdal_itfspscq_recv(r_itf_t obj, void* buffer, int len, int tstamp) {
	RTDAL_ASSERT_PARAM(buffer);
	RTDAL_ASSERT_PARAM(len>=0);

	int n, plen;
	void *ptr=NULL;

	do {
		if ((n = rtdal_itfspscq_pop(obj, &ptr, &plen, tstamp)) < 1) {
			return n;
		}
	} while (n == 2);

	if (plen > len) {
		plen = len;
	}

	memcpy(buffer, ptr, (size_t) plen);

	if ((n = rtdal_itfspscq_release(obj,NULL,0)) != 1) {
		printf("Caution packet could not be released (%d)\n",n);
	}
	return plen;
}



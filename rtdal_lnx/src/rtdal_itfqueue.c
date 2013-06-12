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
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rtdal.h"
#include "rtdal_error.h"
#include "rtdal_itf.h"
#include "rtdal_itfqueue.h"
#include "defs.h"
#include "str.h"

typedef struct {
	int tstamp;
	int len;
}header_t;


#define cast(a,b) RTDAL_ASSERT_PARAM(a);\
			rtdal_itfqueue_t *b = (rtdal_itfqueue_t*) a;
#define cast_p(a,b) RTDAL_ASSERT_PARAM_p(a);\
			rtdal_itfqueue_t *b = (rtdal_itfqueue_t*) a;


int rtdal_itfqueue_init(rtdal_itfqueue_t *itf) {
	RTDAL_ASSERT_PARAM(itf);
	struct mq_attr qattr;

	itf->parent.type = ITF_INT_QUEUE;
	char tmp[64];
	snprintf(tmp,64,"/q-%d-%d",getpid(),itf->parent.id);

	qattr.mq_msgsize = itf->max_msg_sz;
	qattr.mq_maxmsg = itf->max_msg;
	printf("queue name=%s nmsg=%d, msgsize=%d\n",tmp,qattr.mq_maxmsg,qattr.mq_msgsize);
	itf->queue = mq_open(tmp,O_RDWR | O_CREAT | O_EXCL,0644,&qattr);
	if (itf->queue == (mqd_t) -1) {
		perror("mq_open");
		return -1;
	}
	return 0;
}

int rtdal_itfqueue_reset(r_itf_t obj) {
	aerror("Not implemented\n");
	return 0;
}

int rtdal_itfqueue_remove(r_itf_t obj) {
	cast(obj,itf);

	mq_close(itf->queue);

	return 0;
}

int rtdal_itfqueue_request(r_itf_t obj, void **ptr) {
	cast(obj,itf);
	RTDAL_ASSERT_PARAM(ptr);

	int n = posix_memalign(ptr,64,itf->max_msg_sz);
	return n==0?1:-1;
}

int rtdal_itfqueue_push(r_itf_t obj, void *ptr, int len, int tstamp) {
	cast(obj,itf);

	int n;
	if (len>0) {
		n = mq_send(itf->queue,(const char*) ptr,(size_t) len, 0);
	}
	free(ptr);

	return n==0?1:-1;
}


int rtdal_itfqueue_pop(r_itf_t obj, void **ptr, int *len, int tstamp) {
	cast(obj,itf);
	RTDAL_ASSERT_PARAM(ptr);
	RTDAL_ASSERT_PARAM(len);

	void *pkt;
	int n = posix_memalign(&pkt,64,itf->max_msg_sz);
	if (n) {
		return -1;
	}
	n = mq_receive(itf->queue,(char*) pkt,(size_t) len, 0);
	*ptr = pkt;
	
	return n==0?1:-1;
}

int rtdal_itfqueue_release(r_itf_t obj, void *ptr, int len) {
	cast(obj,itf);

	free(ptr);

	return 1;
}

int rtdal_itfqueue_set_delay(r_itf_t obj, int delay) {
	aerror("Not yet implemented");
	return -1;
}

int rtdal_itfqueue_get_delay(r_itf_t obj) {
	aerror("Not yet implemented");
	return -1;
}


int rtdal_itfqueue_set_callback(r_itf_t obj, void (*fnc)(void), int prio) {
	aerror("Not yet implemented");
	return -1;
}

int rtdal_itfqueue_set_blocking(r_itf_t obj, int block) {
	aerror("Not yet implemented");
	return -1;
}

int rtdal_itfqueue_get_blocking(r_itf_t obj) {
	aerror("Not yet implemented");
	return -1;
}


int rtdal_itfqueue_send(r_itf_t obj, void* buffer, int len, int tstamp) {
	cast(obj,itf);
	RTDAL_ASSERT_PARAM(buffer);
	RTDAL_ASSERT_PARAM(len>=0);

	int n;
	void *ptr;

	if (len > itf->max_msg_sz) {
		RTDAL_SETERROR(RTDAL_ERROR_LARGE);
		return -1;
	}

	if ((n = rtdal_itfqueue_request(obj, &ptr)) != 1) {
		return n;
	}

	memcpy(ptr, buffer, (size_t) len);

	return rtdal_itfqueue_push(obj,ptr,len,tstamp);
}

int rtdal_itfqueue_recv(r_itf_t obj, void* buffer, int len, int tstamp) {
	RTDAL_ASSERT_PARAM(buffer);
	RTDAL_ASSERT_PARAM(len>=0);

	int n, plen;
	void *ptr=NULL;

	do {
		if ((n = rtdal_itfqueue_pop(obj, &ptr, &plen, tstamp)) < 1) {
			return n;
		}
	} while (n == 2);

	if (plen > len) {
		plen = len;
	}

	memcpy(buffer, ptr, (size_t) plen);

	if ((n = rtdal_itfqueue_release(obj,ptr,plen)) != 1) {
		printf("Caution packet could not be released (%d)\n",n);
	}

	return plen;
}



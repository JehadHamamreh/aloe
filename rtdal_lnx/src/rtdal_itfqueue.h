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

#ifndef rtdal_ITFqueue_H
#define rtdal_ITFqueue_H

#include "str.h"
#include "rtdal_itf.h"
#include "rtdal.h"
#include <fcntl.h>
#include <mqueue.h>

/* See rtdal_itf.h itflocal_t structure before modifying this */
typedef struct {
	rtdal_itf_t parent;

	int max_msg;
	int max_msg_sz;
	mqd_t queue;
}rtdal_itfqueue_t;


int rtdal_itfqueue_init(rtdal_itfqueue_t *obj);
int rtdal_itfqueue_create(r_itf_t obj, string address);
int rtdal_itfqueue_reset(r_itf_t obj);
int rtdal_itfqueue_remove(r_itf_t obj);
int rtdal_itfqueue_request(r_itf_t obj, void **ptr);
int rtdal_itfqueue_push(r_itf_t obj, void *ptr, int len, int tstamp);
int rtdal_itfqueue_pop(r_itf_t obj, void **ptr, int *len, int tstamp);
int rtdal_itfqueue_release(r_itf_t obj, void *ptr, int len);
int rtdal_itfqueue_send(r_itf_t obj, void* buffer, int len, int tstamp);
int rtdal_itfqueue_recv(r_itf_t obj, void* buffer, int len, int tstamp);
int rtdal_itfqueue_set_callback(r_itf_t obj, void (*fnc)(void), int prio);
int rtdal_itfqueue_set_blocking(r_itf_t obj, int block);
int rtdal_itfqueue_get_blocking(r_itf_t obj);
int rtdal_itfqueue_set_delay(r_itf_t obj, int delay);
int rtdal_itfqueue_get_delay(r_itf_t obj);
#endif

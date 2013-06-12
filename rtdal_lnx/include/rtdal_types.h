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

#ifndef rtdal_TYPES_H_
#define rtdal_TYPES_H_

#include <pthread.h>
#include <sys/time.h>
#define time_t struct timeval

#include "str.h"

struct h_proc_ {
	int id;
};
typedef struct h_proc_* r_proc_t;


struct h_itf_ {
	int id;
	int type;
};
typedef struct h_itf_* r_itf_t;


struct h_dac_ {
	int id;
};
typedef struct h_dac_* r_dac_t;

struct h_log_ {
	int id;
};


typedef enum {
	TEXT, UINT32, INT32
}r_log_mode_t;

typedef struct h_log_* r_log_t;

typedef pthread_t r_task_t;

#endif

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

#ifndef PIPELINE_SYNC_H_
#define PIPELINE_SYNC_H_

#include <pthread.h>
#include <semaphore.h>
#include "futex.h"
#include "objects_max.h"

#define MAX_PIPELINES 20

#define SEM 		1
#define FUTEX 		2
#define BARRIER		3
#define CONDVAR		4

#define PIPELINESYNC_MUTEX_TYPE 4


int pipeline_sync_initialize(int num_pipelines);
void pipeline_sync_thread_waits(int idx);
void pipeline_sync_threads_wake();


#endif

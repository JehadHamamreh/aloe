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

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <error.h>

#include "rtdal.h"
#include "rtdal_timer.h"
#include "futex.h"
#include "defs.h"

extern int timeslot;

void* timer_run_thread(void* x) {
	rtdal_timer_t *obj = (rtdal_timer_t*) x;

	switch(obj->mode) {
	case XENOMAI:
#ifdef __XENO__
		return xenomai_timer_run_thread(obj);
#else
		aerror("Not compiled with xenomai support\n");
		return NULL;
#endif
	case NANOSLEEP:
		return nanoclock_timer_run_thread(obj);
	default:
		aerror_msg("Unknown timer mode %d\n", obj->mode);
		return NULL;
	}
}

inline static void timespec_add_us(struct timespec *t, long int us) {
	t->tv_nsec += us;
	if (t->tv_nsec >= 1000000000) {
		t->tv_nsec = t->tv_nsec - 1000000000;
		t->tv_sec += 1;
	}
}
void timespec_sub_us(struct timespec *t, long int us) {
	t->tv_nsec -= us;
	if (t->tv_nsec < 0) {
		t->tv_nsec = t->tv_nsec + 1000000000;
		t->tv_sec -= 1;
	}
}

void* nanoclock_timer_run_thread(rtdal_timer_t* obj) {
	int s;
	int n;
	assert(obj->period_function);
	

	obj->stop = 0;
	if (obj->wait_futex) {
		futex_wait(obj->wait_futex);
		obj->next.tv_sec+=TIMER_FUTEX_GUARD_SEC;
		obj->next.tv_nsec=0;
	} else {
		clock_gettime(CLOCK_REALTIME, &obj->next);
	}

	n=0;
	while(!obj->stop) {
		timespec_add_us(&obj->next, obj->period);
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,
				&obj->next, NULL);

		timelog(obj->log);

		n++;
		if (n>=obj->multiple) {
			obj->period_function(obj->arg, &obj->next);
			n=0;
		}
	}

	s = 0;
	pthread_exit(&s);
	return NULL;
}

#ifdef __XENO__
void *xenomai_timer_run_thread(rtdal_timer_t *obj) {
	struct timespec period;
	unsigned long overruns;
	int s;
	int n;
	unsigned int tscnt=0;

//	pthread_set_mode_np(0, PTHREAD_WARNSW);
	
	period.tv_sec=0;
	period.tv_nsec=obj->period;
	obj->stop = 0;

	if (obj->wait_futex) {
		futex_wait(obj->wait_futex);
		obj->next.tv_sec+=2;
		obj->next.tv_nsec=0;
	} else {
		clock_gettime(CLOCK_REALTIME, &obj->next);
		obj->next.tv_sec+=2;
	}
	s=pthread_make_periodic_np(pthread_self(), &obj->next, &period);
	if (s) {
		printf("error %d\n",s);
		return NULL;
	}
	n=0;
	while(!obj->stop) {
		overruns=0;
		if ((s=pthread_wait_np(&overruns))) {
			if (s!=110)
				printf("error2 %d\n",s);
		}

		timelog(obj->log);

		n++;
		if (n>=obj->multiple) {
			obj->period_function(obj->arg, &obj->next);
			n=0;
		}
	}
	s=0;
	pthread_exit(&s);
	return NULL;
}
#endif

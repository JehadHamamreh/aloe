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

#include <assert.h>
#include <stdio.h>

#define DEFAULT_LOG_LENGTH	(32*1024)

/* debug rtdal */
extern r_log_t rtdal_log;
#define hdebug(_fmt, ...) do {if (LOGS_ENABLED) \
	rtdal_log_printf(rtdal_log,"[%s(),ts=%d]:\t" _fmt, __func__,rtdal_time_slot(),__VA_ARGS__);}while(0)

/* debug rtdal timing*/
#define timelog(a) do {if (LOGS_ENABLED && a) rtdal_log_add_us(a);}while(0)

/* debug spscq */
#define qdebug(_fmt, ...) \
	do { if (LOGS_ENABLED && itf->parent.log) \
		rtdal_log_printf(itf->parent.log,"[%s(),id=%d,ts=%d]:\t" _fmt, \
				__func__,itf->parent.id,rtdal_time_slot(),__VA_ARGS__);} while(0);


#define WHERESTR  "[%s():%d]: "
#define WHEREARG  __func__, __LINE__
#define DEBUGPRINT2(...)       fprintf(stderr, __VA_ARGS__)
#define aerror_msg(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#define aerror(a)  DEBUGPRINT2(WHERESTR a "\n", WHEREARG)

#define ainfo printf
#define awarn printf

#define aassert(a) if (!(a)) return -1
#define aassert_p(a) if (!(a)) return NULL

#define poserror(a,msg) fprintf(stderr, WHERESTR "%s: \n" msg, WHEREARG, strerror(a))

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#define SUPPRESS_NOT_USED_WARN __attribute__ ((unused))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif


#define CAST(dst,src,type) type dst = (type) src


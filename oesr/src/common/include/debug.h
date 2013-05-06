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

extern r_log_t oesr_log;

#define MODULE_LOG_SZ	(16*1024)
#define OESR_LOG_SIZE	(16*1024)

/* enable measurement of module's execution time*/
#define OESR_API_GETTIME

/* debug memory */
#define DEBUG_POOL 0
#define memdebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_POOL && oesr_log) rtdal_log_printf(oesr_log,"[mempool]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);

/* debug memory */
#define DEBUG_MAPPING 1
#define mapdebug(_fmt, ...) \
		do { if (LOGS_ENABLED && DEBUG_MAPPING && oesr_log) rtdal_log_printf(oesr_log,"[mapping]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);

/* log execution time of modules (OESR_API_GETTIME) must be selected*/
#define tmdebug(a,b) \
	do { if (LOGS_ENABLED && a) rtdal_log_add(a,b,sizeof(int));} while(0);


/* debug packet */
#define DEBUG_PKT 1
#define pktdebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_PKT && oesr_log) rtdal_log_printf(oesr_log,"[packet]\t[%s()]: pkt=0x%x " _fmt, __func__,pkt,__VA_ARGS__);} while(0);


/* debug serializable */
#define DEBUG_SER 1
#define serdebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_SER && oesr_log) rtdal_log_printf(oesr_log,"[serial]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);

/* debug manager */
#define DEBUG_MAN 1
#define mdebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_MAN && oesr_log) rtdal_log_printf(oesr_log,"[manager]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);

/* debug parse */
#define DEBUG_PARSER 1
#define pardebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_PARSER && oesr_log) rtdal_log_printf(oesr_log,"[parser]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);

/* debug node */
#define DEBUG_NODE 1
#define ndebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_NODE && oesr_log) rtdal_log_printf(oesr_log,"[node]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);

/* debug oesr */
#define DEBUG_oesr 1
#define sdebug(_fmt, ...) \
	do { if (LOGS_ENABLED && DEBUG_oesr && oesr_log) rtdal_log_printf(oesr_log,"[api]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0);



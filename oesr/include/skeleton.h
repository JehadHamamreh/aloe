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

#ifndef SKELETON_H
#define SKELETON_H
#include <oesr.h>
#include <assert.h>
#include <stdio.h>

#define MOD_DEBUG 1

#define PRINT_MEX

#ifdef _COMPILE_MEX
	#include "mex.h"
#endif


#ifndef _SKELETON_INCLUDED_CTRL
/** Returns the number of samples (of size input_sample_sz) received from the input port
 * idx.
 * \returns non-negative integer on success or -1 if idx is not a valid interface.
 */
int get_input_samples(int idx);

/** Sets the number of samples (of size output_sample_sz) to send throught the output port
 * idx.
 * \returns 0 on success or -1 on error.
 */
int set_output_samples(int idx, int len);



int work(void **input, void **output);
int initialize();
int stop();
int generate_input_signal(void *input, int *input_length);
#endif

#ifdef _COMPILE_ALOE
	extern void *ctx;
	#define INTERFACE_CONFIG
#endif


/* Info and error messages print */
#ifdef _COMPILE_ALOE
	#define INFOSTR "[info-"
#else
	#define INFOSTR "[info]: "
#endif

#ifdef _COMPILE_ALOE
	#define DEBSTR "[debug-"
#else
	#define DEBSTR "[debug]: "
#endif

#define ERRSTR "[error at "

#ifdef _COMPILE_ALOE
	#define WHERESTR  "%s,ts=%d]: "
	#define WHEREARG  oesr_module_name(ctx),oesr_tstamp(ctx)
#else
	#define WHERESTR  "file %s, line %d]: "
	#define WHEREARG  __FILE__, __LINE__
#endif

#ifdef _COMPILE_MEX
	#ifdef PRINT_MEX
	#define DEBUGPRINT2(...)       mexPrintf(__VA_ARGS__)
	#else
	#define DEBUGPRINT2(...)
	#endif
#elif _COMPILE_ALOE
	#define DEBUGPRINT2(...)       	oesr_printf(ctx,__VA_ARGS__)
#else
	#define DEBUGPRINT2(...)		fprintf(stdout,__VA_ARGS__)
#endif


#define aerror_msg(_fmt, ...)  		DEBUGPRINT2(ERRSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#define aerror(a)  					DEBUGPRINT2( ERRSTR WHERESTR a, WHEREARG)

#ifdef _COMPILE_ALOE
	#define ainfo(a) 				DEBUGPRINT2(INFOSTR WHERESTR a, WHEREARG)
	#define ainfo_msg(_fmt, ...)  	DEBUGPRINT2(INFOSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#else
	#define ainfo(a) 				DEBUGPRINT2(INFOSTR a)
	#define ainfo_msg(_fmt, ...)  	DEBUGPRINT2(INFOSTR _fmt, __VA_ARGS__)
#endif

#ifdef _COMPILE_ALOE
	#define adebug_msg(_fmt, ...)  	DEBUGPRINT2(DEBSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#else
	#define adebug_msg(_fmt, ...)  	DEBUGPRINT2(DEBSTR _fmt, __VA_ARGS__)
#endif

#define moderror 		aerror
#define moderror_msg 	aerror_msg


#ifdef _COMPILE_ALOE
	#define modinfo(a) \
		do { if (LOGS_ENABLED && oesr_log_level(ctx)&LOG_LEVEL_INFO) \
			ainfo(a); } while(0);
	#define modinfo_msg(_fmt, ...) \
		do { if (LOGS_ENABLED && oesr_log_level(ctx)&LOG_LEVEL_INFO) \
			ainfo_msg(_fmt,__VA_ARGS__); } while(0);
#else
	#define modinfo 		ainfo
	#define modinfo_msg 	ainfo_msg
#endif


#ifdef _COMPILE_ALOE
	#define itflog(port,mode,samples,bytes) \
		do { if (LOGS_ENABLED && MOD_DEBUG && (oesr_log_level(ctx)&LOG_LEVEL_ITF)) \
			oesr_printf(ctx,"[%s,ts=%d]: port=%d, %s=%d samples (%d bytes)\n",\
					oesr_module_name(ctx),oesr_tstamp(ctx),port,mode,samples,bytes); } while(0)
#else
#define itflog(port,mode,samples,bytes) \
		do { if (MOD_DEBUG) \
			printf("[itf]: port=%d, %s=%d samples (%d bytes)\n",\
				port,mode,samples,bytes); } while(0)
#endif

#ifdef _COMPILE_ALOE
	#define moddebug(_fmt, ...) \
		do { if (LOGS_ENABLED && MOD_DEBUG && (oesr_log_level(ctx)&LOG_LEVEL_DEBUG)) \
			adebug_msg(_fmt,__VA_ARGS__); } while(0)
#else
	#define moddebug(_fmt, ...) \
		do { if (MOD_DEBUG) \
				printf("[debug]\t[%s()]: " _fmt, __func__,__VA_ARGS__);} while(0)
#endif

#endif


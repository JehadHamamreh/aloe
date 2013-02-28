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

#define USE_LOG 0
#define MOD_DEBUG 0
#define ITF_DEBUG 0

#define CTRL_PKT_VALUE_SZ	128

struct ctrl_in_pkt {
	int pm_idx;
	int size;
	char value[CTRL_PKT_VALUE_SZ];
};

/*#define PRINT_MEX
*/

/*#define DEBUG_TRACE
*/
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


#ifdef _COMPILE_MEX
	#include "mex.h"
#endif


int work(void **input, void **output);
int initialize();
int stop();
int generate_input_signal(void *input, int *input_length);


#ifdef _COMPILE_ALOE
	extern void *ctx;
	extern log_t mlog;
	#define INTERFACE_CONFIG
#endif



#ifdef DEBUG_TRACE
	#define _DEBUG_TRACE
	#ifdef _COMPILE_ALOE
		extern FILE *trace_buffer;
	#else
		#define trace_buffer stdout
	#endif

	#define debug_buffer (trace_buffer?trace_buffer:stdout)
#else
	#define debug_buffer stdout
#endif




/* Info and error messages print */
#ifdef _COMPILE_ALOE
	#define INFOSTR "[info at "
#else
	#define INFOSTR "[info]: "
#endif

#define ERRSTR "[error at "

#ifdef _COMPILE_ALOE
	#define WHERESTR  "%s]: "
	#define WHEREARG  oesr_module_name(ctx)
#else
	#define WHERESTR  "file %s, line %d]: "
	#define WHEREARG  __FILE__, __LINE__
#endif

#ifdef _COMPILE_MEX
	#ifdef PRINT_MEX
	#define DEBUGPRINT2(out,...)       mexPrintf(__VA_ARGS__)
	#else
	#define DEBUGPRINT2(out,...)
	#endif
#elif _COMPILE_ALOE
	#define DEBUGPRINT2(out,...)       if (mlog && USE_LOG) { oesr_log_printf(mlog,__VA_ARGS__); }\
						else {fprintf(out, __VA_ARGS__); }
#else
	#define DEBUGPRINT2(out,...)	fprintf(out,__VA_ARGS__)
#endif


#define aerror_msg(_fmt, ...)  DEBUGPRINT2(debug_buffer,ERRSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#define aerror(a)  DEBUGPRINT2(stderr, ERRSTR WHERESTR a, WHEREARG)

#ifdef _COMPILE_ALOE
	#define ainfo(a) DEBUGPRINT2(debug_buffer, INFOSTR WHERESTR a, WHEREARG)
	#define ainfo_msg(_fmt, ...)  DEBUGPRINT2(debug_buffer,INFOSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#else
	#define ainfo(a) DEBUGPRINT2(debug_buffer, INFOSTR a)
	#define ainfo_msg(_fmt, ...)  DEBUGPRINT2(debug_buffer,INFOSTR _fmt, __VA_ARGS__)
#endif

#define modinfo 		ainfo
#define modinfo_msg 	ainfo_msg
#define moderror 		aerror
#define moderror_msg 	aerror_msg

#ifdef _COMPILE_ALOE
	#define moddebug(_fmt, ...) \
		do { if (MOD_DEBUG) fprintf(debug_buffer,"[mod_debug-%s]\t[%s()]: ts=%d " _fmt, oesr_module_name(ctx),__func__,\
				oesr_tstamp(ctx),__VA_ARGS__);} while(0);
	#define itfdebug(_fmt, ...) \
		do { if (ITF_DEBUG) fprintf(debug_buffer,"[itf_debug-%s]\t[%s()]: ts=%d " _fmt, oesr_module_name(ctx),__func__,\
			oesr_tstamp(ctx),__VA_ARGS__);} while(0);
#else
	#define moddebug(_fmt, ...) \
		do { if (MOD_DEBUG) fprintf(debug_buffer,"[mod_debug]\t[%s()]: " _fmt, __func__,\
				__VA_ARGS__);} while(0);
#endif

#endif


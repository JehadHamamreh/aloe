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

/** Sends size bytes from the buffer value to the destination variable given by the index
 * dest_idx in the structure remote_params_db_t remote_params_db[]
 * \returns 0 on success, or -1 on error
 */
int ctrl_skeleton_send_idx(int dest_idx, void *value, int size, int tstamp);

/** Sends size bytes from the buffer value to the destination variable given by
 * module_name and variable_name
 * \returns 0 on success, or -1 on error
 */
int ctrl_skeleton_send_name(char *module_name, char *variable_name, void *value, int size, int tstamp);

/* This function is called each timeslot, regardless of the parameters having changed or not.
 *
 * \param tslot Value of the current timeslot
 * \returns 0 on success or -1 on error
 */
int ctrl_work(int tslot);

int ctrl_init();


/** value and size fields may be set to NULL and zero if ctrl_send_always is zero
 */
typedef struct {
	const char *module_name;
	const char *variable_name;
	void *value;
	int size;
}remote_params_db_t;

typedef struct {
	const char *name;
	void *value;
	int size;
} my_params_db_t;

#define MAX_VARIABLES 200




/* Info and error messages print */
#define debug_buffer stdout
#define INFOSTR "[info at "
#define ERRSTR "[error at "

#define WHERESTR  "%s]: "
#define WHEREARG  oesr_module_name(ctx)

#define DEBUGPRINT2(out,...)	fprintf(out,__VA_ARGS__)

#define aerror_msg(_fmt, ...)  DEBUGPRINT2(debug_buffer,ERRSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#define aerror(a)  DEBUGPRINT2(stderr, ERRSTR WHERESTR a, WHEREARG)

#define ainfo(a) DEBUGPRINT2(debug_buffer, INFOSTR WHERESTR a, WHEREARG)
#define ainfo_msg(_fmt, ...)  DEBUGPRINT2(debug_buffer,INFOSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)

#define modinfo 		ainfo
#define modinfo_msg 	ainfo_msg
#define moderror 		aerror
#define moderror_msg 	aerror_msg

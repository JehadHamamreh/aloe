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

int set_remote_params(remote_params_db_t *params);

/* This function is called each timeslot, regardless of the parameters having changed or not.
 *
 * \param tslot Value of the current timeslot
 * \returns 0 on success or -1 on error
 */
int ctrl_work(int tslot);

int ctrl_init();

#define _SKELETON_INCLUDED_CTRL
#include "skeleton.h"


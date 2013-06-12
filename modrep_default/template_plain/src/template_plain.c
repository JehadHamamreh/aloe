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

#include <stdio.h>
#include <oesr.h>
#include "template_plain.h"

/** @ingroup template_plain
 * Return 1 on success, 0 to be called in the next timeslot or -1 to stop the waveform
 */
int Init(void *context) {
	return 1;
}

/** @ingroup template_plain
 * Return 0 on success or -1 to stop the waveform
 */
int Run(void *context) {
	return 0;
}

/** @ingroup template_plain
 *  Return 0 on success or -1 on error
 */
int Stop(void *context) {
	return 0;
}

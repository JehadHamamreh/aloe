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

#ifndef rtdal_DAC_H
#define rtdal_DAC_H

#include "rtdal.h"

#define MAX_DAC_HANDLERS 5

/**
 * This is the interface to the digital converters.
 */
typedef struct {
	int id;
	void *handler;
	int code;
} rtdal_dac_t;

struct dac_names_ {
	const char *name;
	int code;
};

#define DAC_USRP 1

const static struct dac_names_ dac_names[] = {
#ifdef HAVE_UHD
		{"USRP",DAC_USRP},
#endif
		{NULL,0}};

#ifdef HAVE_UHD
#include "uhd/uhd.h"
#define call_usrp(a,...)  uhd_##a(__VA_ARGS__);
#else
#define call_usrp(a,...) -1
#endif


#endif

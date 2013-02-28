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



/**
 * @defgroup lte_ctrl_tx lte_ctrl_tx
 * Document here your module
 * @{
 */
#ifndef DEFINE_H
#define DEFINE_H

typedef struct ctrl_in_pkt output_t;

#define OUTPUT_MAX_SAMPLES 	1

#define NOF_OUTPUT_ITF		1

#endif

/**@} */



/********* do not need to modify beyond here */


#ifndef INCLUDE_DEFS_ONLY

/* Input and output buffer sizes (in number of samples) */
const int input_max_samples = 0;
const int output_max_samples = OUTPUT_MAX_SAMPLES;

/* leave these two lines unmodified */
const int input_sample_sz = 0;
const int output_sample_sz = sizeof(output_t);

/* Number of I/O interfaces. All have the same maximum size */
const int nof_input_itf = 0;
int nof_output_itf = NOF_OUTPUT_ITF;

#endif

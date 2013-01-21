/* 
 * Copyright (c) 2012, Xavier Arteaga, Antoni Gelonch <antoni@tsc.upc.edu> &
 * Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
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
 * @defgroup template template
 * Document here your module
 * @{
 */
#ifndef DEFINE_H
#define DEFINE_H

typedef _Complex float input_t;
typedef _Complex float output_t;

#define OUTPUT_MAX_SAMPLES 	14*2048
#define INPUT_MAX_SAMPLES 	14*2048

#define NOF_INPUT_ITF		1	/**TO BE MODIFIED*/
#define NOF_OUTPUT_ITF		1

#endif

#define CELLID 			0	/** Values from 0 to 503*/
#define MAXFFTSIZE 		2048	// 128, 256, 512, 1024, 1536, 2048
#define MAXNUMSUBC 		1200	// Number of subcarriers: 72, 180, 300, 600, 900, 1200
#define MAXNUMOFDM		140 	// Number of OFDM symbols: 140 (CP normal), 120 (CP extended)
#define MAXNOFSYMBOLSSLOT	7	// Max number of OFDM symbols per slot
#define CPNORMAL 		0
#define CPEXTENDED 		1
#define NUMSLOTSxFRAME		20

/**@} */
#define GENERATE_COMPLEX


#ifndef INCLUDE_DEFS_ONLY

/* Input and output buffer sizes (in number of samples) */
const int input_max_samples = INPUT_MAX_SAMPLES;
const int output_max_samples = OUTPUT_MAX_SAMPLES;

/* leave these two lines unmodified */
const int input_sample_sz = sizeof(input_t);
const int output_sample_sz = sizeof(output_t);

/* Number of I/O interfaces. All have the same maximum size */
const int nof_input_itf = NOF_INPUT_ITF;
const int nof_output_itf = NOF_OUTPUT_ITF;

#endif

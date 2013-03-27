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


/* Maximum number of integers containing the scrambling sequence bits */
#define L_MAX			32	/* scrambling sequence is a single integer! */
# define L_default		16

/* Channel types where CRC scrambling applies */
#define PDCCH			0
#define PBCH			1

#define channel_default		0

struct pdcch_params {
	unsigned rnti;
	int antenna_selection;
	int ue_port;
	int dci_format;
};

/* Function prototypes */
int verify_initialization_parameters(void);
int verify_runtime_parameters(unsigned rnti, int ue_port);
void srambling(char *input, char *output, int N, char *c);
void desrambling(float *input, float *output, int N, char *c);
void pdcch_sequence_gen(char *c, struct pdcch_params);
void pbch_sequence_gen(char *c, int nof_ports);

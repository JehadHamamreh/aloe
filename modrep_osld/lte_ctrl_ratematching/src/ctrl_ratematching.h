/*
 * Copyright (c) 2012, Vuk Marojevic <marojevic@tsc.upc.edu>.
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

#define vx_MAX_size 	1000
#define DUMMYBIT	2

/* Returns the result of floor(X/Y) and ceil(X/Y), X, Y: integers */
#define intfloor(X, Y) 	(X/Y)
#define intceil(X, Y) 	((X-1)/Y+1)
#define COLS 32
#define ROWS intceil(inlen, COLS)

#ifndef INCLUDE_FUNCTIONS
/* Permutation pattern */
const static int PERM[32] = {1, 17, 9, 25, 5, 21, 13, 29,\
		3, 19, 11, 27, 7, 23, 15, 31, \
		0, 16, 8, 24, 4, 20, 12, 28, \
		2, 18, 10, 26, 6, 22, 14, 30};

#endif

/* Function prototypes */
void rate_matching(char *input, char *output, int in_l, int E);
int rate_unmatching(float *input, float *output, int in_l, int S);

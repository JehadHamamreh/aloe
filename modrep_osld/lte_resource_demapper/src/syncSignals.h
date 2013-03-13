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

#ifndef _SYNCSIGNALS_H
#define _SYNCSIGNALS_H

#define PI 3.14159265

#define MAXPHYLAYERCELLID 504	/** Max value for the Physical Layer Cell-ID*/

#define PSSLEN 		62
#define PSSCELLID0 	25.0
#define PSSCELLID1 	29.0
#define PSSCELLID2 	34.0

#define SSSLEN 124
#define NUMSSS 168

int setPSS(int phylayerID, _Complex float *PSSsymb, int TxRxMode);
void loadmtable (int *m0s, int *m1s);
int setSSS (int cellid, float *SSSseq, int *m0s, int *m1s);

#endif

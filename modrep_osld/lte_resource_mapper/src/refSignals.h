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


#ifndef _REFSIGNALS_H
#define _REFSIGNALS_H

/* maximum expected reference signals in the frame */
#define MAXRS (1200*140*4/12/7)
#define Nc 1600
#define GOLDMAXLEN (Nc+MAXRS+128)
#define GOLDINITLEN 31

#define Cinit ((2<<9)*(7*(ns+1)+l+1)*(2*phyLayerCellID+1)+2*phyLayerCellID+cyclic)
#define Cn(X) ((x1[X+Nc]+x2[X+Nc])%2)
#define NUMSLOTS (numOFDM/7)

/** Global Variables*/
_Complex float CRSsymb[MAXRS];

int setCRefSignals (int numOFDM, int numDL, int phyLayerCellID, _Complex float *CRS);

#endif

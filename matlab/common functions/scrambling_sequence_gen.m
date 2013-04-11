% /* 
%  * Copyright (c) 2012-2013, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
%  * This file is part of ALOE++ (http://flexnets.upc.edu/)
%  * 
%  * ALOE++ is free software: you can redistribute it and/or modify
%  * it under the terms of the GNU Lesser General Public License as published by
%  * the Free Software Foundation, either version 3 of the License, or
%  * (at your option) any later version.
%  * 
%  * ALOE++ is distributed in the hope that it will be useful,
%  * but WITHOUT ANY WARRANTY; without even the implied warranty of
%  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  * GNU Lesser General Public License for more details.
%  * 
%  * You should have received a copy of the GNU Lesser General Public License
%  * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
%  */


% Generation of scrambling sequence %


% Inputs:
% q         - codeword index (integer [0, 1]). In case of single codeword:
%           q=0
%
% M         - Number of bits per codeword and subframe
%
% cell_gr   - Cell ID group index (integer [0,2])
%
% cell_sec  - cell ID sector index within the physical-layer
%           cell-identity group (integer [0, 167])
%
% n_RNTI	- Radio network temporary identifier (integer [0, 2e16-1=65535])
%
% N_MBSFN	- Parameter used for determining the scrambling sequence of the
% PMCH
%
% ns        - Slot numer: 0, 1, ..., 19
%
% Outputs:
% c         - scrambling sequence of length M
%


% Spec:         3GPP TS 36.211 section 6.3.1 v10.5.0
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:30.1.2013
% 

% Scrambling sequence is a function of the subframe 
% Returns the scrambling sequence of length M
function c = scrambling_sequence_gen(q, M, cell_gr, cell_sec, nRNTI, N_MBSFN, ns, channel)

Nc = 1600;

% There are 504 unique physical-layer cell identities (N_cell). The physical-layer 
% cell identities are grouped into 168 unique physical-layer cell-identity 
% groups (cell_gr [0..167]). Each group contains three unique identities, indicated 
% by the physical-layer identity within the physical-layer cell-identity 
% group (cell_sec [0..2]).
N_cell = 3*cell_gr + cell_sec;


x1 = zeros(1,M+Nc);
% The first m-sequence {x1} should be initialized with
x1(1) = 1;
n=2:31;
x1(n) = zeros(1,30);

% The initialization of the second m-sequence {x2} is denoted by c_init
if (channel == 0)      % PDSCH or PULSCH
    c_init = nRNTI*2^14 + q*2^13 + floor(ns/2)*2^9 + N_cell;

elseif (channel == 1)  % PCFICH
    c_init = (floor(ns/2)+1) * (2*N_cell+1)*2^9 + N_cell;

elseif (channel == 2)  % PDCCH
    c_init = floor(ns/2)*2^9 + N_cell;

elseif (channel == 3)  % PBCH
    c_init = N_cell;  % at each radio frame fulfilling (nf mod 4) = 0, where nf is the system frame number

elseif (channel == 4)  % PMCH
    c_init = floor(ns/2)*2^9 + N_MBSFN;
     
elseif (channel == 5)  % PUCCH, formats 2, 2a, 2b, 3
    c_init = (floor(ns/2)+1) * (2*N_cell+1)*2^16 + nRNTI; % nRNTI = C-RNTI
end
% at the start of each subframe, where n_RNTI corresponds to the RNTI 
% associated with the PUSCH or PDSCH transmission.
% ns: slot number within a radio frame: 0..19
% c_init changes on even slots--floor(ns/2)--, that is on subframe basis

x2 = zeros(1,M+Nc);
% compute x2{} initialization values:
for i=1:31
    if (bitand(c_init,2^(i-1)) > 0)
        x2(i) = 1;
    end
end

for n=1:M+Nc
    x1(n+31) = mod(x1(n+3) + x1(n), 2);
    x2(n+31) = mod(x2(n+3) + x2(n+2) + x2(n+1) + x2(n), 2);
end

n=1:M;
c(n) = mod(x1(n+Nc) + x2(n+Nc), 2);

end

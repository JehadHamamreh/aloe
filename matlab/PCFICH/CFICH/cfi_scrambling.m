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

% Control Format Indicator (CFI) coding. Determines the 32-bit sequence of
% as a function of the CFI.

%% Parameters
% Inputs:
% cfi           - 
%
%
% Outputs:
% out           - 
%

% Spec:         3GPP TS 36.212, v10.6.0 section 5.3.4.1
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:18.2.2013
% 

function out = cfi_scrambling(in, cell_gr, cell_sec, ns)

    addpath('/home/vuk/DATOS/work/OSLD/scrambling');
    channel = 1; % PCFICH
    M = length(in);
    c = scrambling_sequence_gen(0, M, cell_gr, cell_sec, 0, 0, ns, channel);

    out = mod(in+c,2);
 
end

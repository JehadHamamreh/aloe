% /* 
%  * Copyright (c) 2012-2013, Vuk Marojevic <vuk.marojevic@gmail.com>.
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
% in            - input samples: bit-sequence
%
% cell_gr       - % Physical-layer cell-identity group, range: integer in
%               [0, 167]
%
% cell_sec      - % Physical-layer identity within the physical-layer
%               identity group, range: integer in [0, 2]
%
% Outputs:
% out           - output samples: scrambled input bit-sequence
%

% Spec:         3GPP TS 36.212, v10.6.0 section 5.3.4.1
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:18.2.2013
% 

function out = dci_scrambling(in, cell_gr, cell_sec, ns, desc)

    addpath('../common functions');
    channel = 2; % PDCCH
    M = length(in);
    out = zeros(1,M);
    
    c = scrambling_sequence_gen(0, M, cell_gr, cell_sec, 0, 0, ns, channel);

    if (desc)   % descrambling of soft bits
       for i=1:M
           if (((in(i) > 0) && (c(i) == 1)) || ((in(i) < 0) && (c(i) == 1)))
                out(i) = -in(i);
            else
                out(i) = in(i);
            end
       end     
    else    % scrambling
        out = mod(in+c,2);
    end
 
end

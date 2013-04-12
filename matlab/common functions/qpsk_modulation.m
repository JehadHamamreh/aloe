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

% Spec:         3GPP TS 36.211, v10.5.0 section 7.1.2
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:20.2.2013
% 


%% Parameters
% Inputs:
% in            - Bit sequence
%
%
% Outputs:
% out           - Complex modulation symbols (QPSK)
%

function out = qpsk_modulation(in)
    
    M = length(in);
    if (M == 0)
        out = [];
        return;
    end
    
    N = floor(M/2);

    if (mod(M,2) > 0)
        fprintf('\nWarning: Number of bits not integer divisible by 2. Discarding last bit.\n');
    end
    out = zeros(1,N);
    
    QPSK_level = 1/sqrt(2);
    
    for i=1:N
        if (in(2*i-1) == 0)     % in C: 2*i
            if (in(2*i) == 0)   % in C: 2*i+1
                out(i) = QPSK_level + 1i*QPSK_level;
            else
                out(i) = QPSK_level - 1i*QPSK_level;
            end
        else
            if (in(2*i) == 0)
                out(i) = -QPSK_level + 1i*QPSK_level;
            else
                out(i) = -QPSK_level - 1i*QPSK_level;
            end
        end
    end
    
end

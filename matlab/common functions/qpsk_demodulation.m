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

% QPSK demodulation, hard

% Spec:         (3GPP TS 36.211, v10.5.0 section 7.1.2)
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:20.2.2013
% 


%% Parameters
% Inputs:
% in            - Complex modulation symbols (QPSK)
%
%
% Outputs:
% out           - Bit sequence
%

function out = qpsk_demodulation(in)
    
    M = length(in);
    out = zeros(1,2*M);
    
    for i=1:M
        if (real(in(i)) > 0)
            if (imag(in(i)) > 0)
                out(2*i-1) = 0;
                out(2*i) = 0;
            else
                out(2*i-1) = 0;
                out(2*i) = 1;
            end
        else
            if (imag(in(i)) > 0)
                out(2*i-1) = 1;
                out(2*i) = 0;
            else
                out(2*i-1) = 1;
                out(2*i) = 1;
            end
       end
    end
    
end

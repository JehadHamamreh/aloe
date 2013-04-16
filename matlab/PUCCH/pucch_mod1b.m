% /* 
%  * Copyright (c) 2012-2013, Vuk Marojevic <marojevic@tsc.upc.edu>.
%  * This file is part of ALOE++ (http://flexnets.upc.edu/)
%  * 
%  * ALOE++ is free software: you can redistribute it and/or modify
%  * it under the terms of the GNU Lesser General Public License as published by
%  * the Free Software Foundation, either version 3 of the License, or
%  * (at your option) any later version.
%  * 
%  * ALOE++ is distributed in the hope that it will be useful,
%  * but WITHOUT ANY WARRANTY; without even the implied warranty o
%  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  * GNU Lesser General Public License for more details.
%  * 
%  * You should have received a copy of the GNU Lesser General Public License
%  * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
%  */

% Physical Uplink Control Channel (PUCCH) modulation

% Reference: 3GPP TS 136 211, V 10.5.0, Section 5.4



function out = pucch_mod1b(in, direction)

    % Symbolic constants
    TX = 1;
    RX = 0;
    
    in_l = length(in);

    if (direction == TX)
        if (in_l ~= 2)
            fprintf('\nWrong number of bits %d for format 1b. Should 2.', in_l);
            return;
        end
        out = zeros(1,1);
        if ((in(1) == 0) && (in(2) == 0))
        	out = 1;
        elseif ((in(1) == 0) && (in(2) == 1))
            out = -1j;
        elseif ((in(1) == 1) && (in(2) == 0))
            out = 1j;
        else % "11"
            out = -1;
        end
        
    else    % Rx
        if (in_l ~= 1)
            fprintf('\nWrong number of bits %d for format 1b. Should 1.', in_l);
            return;
        end
        out = zeros(1,2);
        if (real(in(1)) >= abs(imag(in(1))))
        	out = [0 0];
        elseif (-real(in(1)) >= abs(imag(in(1))))
        	out = [1 1];
        elseif (imag(in(1)) >= abs(real(in(1))))
        	out = [1 0];
        elseif (-imag(in(1)) >= abs(real(in(1))))
        	out = [0 1];
        end
    end
    
end

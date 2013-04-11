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
%  * but WITHOUT ANY WARRANTY; without even the implied warranty of
%  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  * GNU Lesser General Public License for more details.
%  * 
%  * You should have received a copy of the GNU Lesser General Public License
%  * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
%  */


% Bit collection, part of the PDCCH Rate Matching


% Inputs:
% w         - Input bit stream (stream of '0s' and '1s'), which 
%           is the output streams of the PDCCH Rate Matching bit collector
%
% Outputs:
% out       - single output bit stream, after bit slection and pruning of 
%           input stream


function e = bit_selection(w, E)

	Kw = length(w);
    e = zeros(1,E);
       
    k = 1;
    for j=1:5   % 2 loops should be enough, 5 just in case (at the 4th the coding rate of 1/3 would be eliminated)
        for i=1:Kw
            if (w(i) ~= -1)   % '-1' indicates dummy/<NULL>-bit
                e(k) = w(i);
                if (k<E)
                    k = k+1;
                else
                    return;
                end
            end
        end
    end
end

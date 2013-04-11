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


% Bit collection, part of the PDCCH Rate Unmatching


% Inputs:
% w         - Input bit stream (stream of '0s' and '1s'), which 
%           is the output streams of the PDCCH Rate Matching bit collector
%
% Outputs:
% out       - single output bit stream, after bit slection and pruning of 
%           input stream


function w = bit_deselection_zeros(e, S)

	E = length(e);
    w = zeros(1,S);
       
    %ex = [e, e, e, e, e];   % emulate a circular buffer, assuming S <= 5*E
    if (E < S)
        w(1:E) = e;
    else
        w = e(1:S);
    end
    
end

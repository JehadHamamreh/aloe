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


function v = bit_split(w)

	Kw = length(w);
    Kp = Kw/3;
    if (mod(Kw,3)~=0)
        fprintf('\nSomething went wrong. RM input stream not divisible by 3.\n');
    end
    
	v = [w(1:Kp); w(Kp+1:2*Kp); w(2*Kp+1:3*Kp)];   % split the input stream into 3 streams
        
end

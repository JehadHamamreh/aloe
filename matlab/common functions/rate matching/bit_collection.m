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
% v0, v1, v2    - 3 input bit streams (3 streams of '0s' and '1s'), which 
%           are the 3 output streams of PDCCH Rate Matching Interleaver 
%
% Outputs:
% out       - single output bit stream, concatenation of the 3 input
%           streams


function out = bit_collection(v0, v1, v2)

	%Kp = length(v0);    % all three subblocks have the same length
	%Kw = 3*Kp;          % length of w, collecting the three bit-streams
        
	out = [v0, v1, v2];   % concatenate the input streams
        
end

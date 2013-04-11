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


% Wraps input streams into a single output stream


% Inputs:
% input     - Input bit streams. The number of rows is the number of
%           streams to be wrapped into a signle stream
%
% Outputs:
% out       - output stream, single stream of length S


function out = wrap_input(input)

        [nof_streams, D] = size(input);
        S = D*nof_streams;
        
        out = zeros(1,S);
        i=0:S-1;
        for j=1:nof_streams
            %nof_streams*i+j
            out(j:nof_streams:S) = input(j,:);
        end
 end

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


% Unwraps input stream into nof_streams output streams


% Inputs:
% input     - Input bit stream
%
% nof_streams   - Number of output streams that the input stream should be
%           unwrapped
%
% Outputs:
% out       - Output streams


function out = unwrap_input(input, nof_streams)

        S = length(input);

        if (mod(S,nof_streams) > 0)
            fprintf('\nError: Input streams should be integer dividible by %d.n', nof_streams);
            return;
        end
        D = S/nof_streams;
        
        out = zeros(nof_streams,D);
        i=0:D-1;
        for j=1:nof_streams
            %nof_streams*i+j
            out(j,:) = input(nof_streams*i+j*ones(1,D));
        end
 end

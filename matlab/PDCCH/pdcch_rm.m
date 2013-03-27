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


% Rate Matching for PDCCH


% The rate matching block creates an output bitstream with a desired code 
% rate. The three bitstreams from the convolutional encoder are interleaved 
% followed by bit collection to create a circular buffer. Bits are selected 
% and pruned from the buffer to create an output bitstream with the desired 
% code rate.


% Inputs:
% in        - Input bit streams (stream of '0s' and '1s'), which is the
%           output of the encoder
%
% Outputs:
% out       - output stream


function out = pdcch_rm(input, turbo, E)

        S = length(input);

        if (mod(S,3) > 0)
            fprintf('\nError: Input streams should be integer dividible by 3, being the output of a 1/3 code rate convolutional or turbo encoder.n');
            return;
        end
        D = S/3;
        
        d = unwrap_input(input,3);
        
        % Subblock Interleaving
        v0 = subblock_interleaver(d(1,:), 0, turbo);
        v1 = subblock_interleaver(d(2,:), 1, turbo);
        v2 = subblock_interleaver(d(3,:), 2, turbo);       
        
        % Bit collection
        w = bit_collection(v0, v1, v2);
        
        % Bit selection and pruning
        e = bit_selection(w, E);
        
        out = e;

end

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


% Un-rate Matching for PDCCH


% The rate matching block creates an output bitstream with a desired code 
% rate. The three bitstreams from the convolutional encoder are interleaved 
% followed by bit collection to create a circular buffer. Bits are selected 
% and pruned from the buffer to create an output bitstream with the desired 
% code rate.


% Inputs:
% e         - Input bit stream
%
% turbo     - If set, assumes the rate matching of turbo encoded
%           information (data channels, such as PDSCH)
%
% S         - Output length
%
% Outputs:
% out       - Output streams of rate matching in reception mode


function out = pdcch_unrm(e, turbo, S)

        if (mod(S,3) > 0)
            fprintf('\nError: Input streams should be integer dividible by 3.n');
            return;
        end
        %D = S/3;

        % Bit deselection and expansion/pruning
        w = bit_deselection_zeros(e, S);
        
        % Bit split
        v = bit_split(w);
        
        % Subblock Deinterleaving
        d0 = subblock_deinterleaver(v(1,:), turbo);
        d1 = subblock_deinterleaver(v(2,:), turbo);
        d2 = subblock_deinterleaver(v(3,:), turbo);       
        
        d = [d0;d1;d2];
        out = wrap_input(d);

end

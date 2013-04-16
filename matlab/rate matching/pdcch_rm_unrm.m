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
% in        - Input bit streams (3 streams of '0s' and '1s'), which are the
%           3 output streams of the encoder, 3xD-array of bits
%
% Outputs:
% out       - output stream

% Conv. Coding parameters and output
D = 106; % # bits of each conv. enconder stream (total of 3*M bits per PDCCH and subframe)
input = [(randi(2,1,D)==1), (randi(2,1,D)==1), (randi(2,1,D)==1)];   % the output bits stream of the conv. enconder
nof_streams = 3;
turbo = 0;
%E = 338;        % # RM output bits
E = 300;        % # RM output bits
S = 3*D;
direct = 1;

%function out = pdcch_rm_unrm(d, turbo, E)

    addpath('../common functions');
    
    %if (direct)
        
    	rm_out = pdcch_rm(input, turbo, E);      % Tx
        unrm_out = pdcch_unrm(rm_out, turbo, S); % Rx
        
    %else
            
    	% ****** TX ******
        % Unwrpas input stream into nof_streams output streams
        d = unwrap_input(input, nof_streams)

        % Subblock Interleaving
        v0 = subblock_interleaver(d(1,:), 0, turbo)
        v1 = subblock_interleaver(d(2,:), 1, turbo)
        v2 = subblock_interleaver(d(3,:), 2, turbo)        
        
        % Bit collection
        w = bit_collection(v0, v1, v2);
        
        % Bit selection and pruning
        e = bit_selection(w, E);

        % ****** RX ******
        % Bit deselection and expansion/pruning
        w = bit_deselection_zeros(e, S);
        
        % Bit split
        v = bit_split(w);
        
        % Subblock Deinterleaving
        d0 = subblock_deinterleaver(v(1,:), turbo)
        d1 = subblock_deinterleaver(v(2,:), turbo)
        d2 = subblock_deinterleaver(v(3,:), turbo)        
        
        d = [d0; d1; d2];
        un_e = wrap_input(d);
    
    %end
%end

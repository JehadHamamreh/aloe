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

addpath('/home/vuk/DATOS/workspace/lte_ctrl_ratematching');

% Conv. Coding parameters and output
D = 500; % # bits of each conv. enconder stream (total of 3*M bits per PDCCH and subframe)
E = 338;        % # RM output bits
in_tx = [(randi(2,1,D)==1), (randi(2,1,D)==1), (randi(2,1,D)==1)];   % the output bits stream of the conv. enconder
in_rx = zeros(1,E);
for i=1:E
    in_rx(i) = 0.1*i;
end

nof_streams = 3;
turbo = 0;
S = 3*D;


rm_out = pdcch_rm(in_tx, turbo, E);      % Tx
rm_mex = am_lte_ctrl_ratematching(in_tx, {{'direction',int32(0)},{'E',int32(E)}});

unrm_out = pdcch_unrm(in_rx, turbo, S); % Rx
unrm_mex = am_lte_ctrl_ratematching(in_rx, {{'direction',int32(1)},{'S',int32(S)}});
        

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
%  * but WITHOUT ANY WARRANTY; without even the implied warranty o
%  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  * GNU Lesser General Public License for more details.
%  * 
%  * You should have received a copy of the GNU Lesser General Public License
%  * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
%  */

% Uplink Control Information (UCI) on the LTE Physical Uplink Control Channel (PUCCH)

% Data arrives to the coding unit in the form of indicators for measurement 
% indication, scheduling request and HARQ acknowledgement.

% Three forms of channel coding are used, one for the channel quality 
% information CQI/PMI, another for HARQ-ACK (acknowledgement) and scheduling 
% request and another for combination of CQI/PMI and HARQ-ACK.

% Reference: 3GPP TS 136 212, V 10.6.0
% mode 0: Channel coding for UCI HARQ-ACK, Section 5.2.3.1 -- not implemented
% mode 1: Channel coding for UCI, Section 5.2.3.2 -- not yet implemented
% mode 2: Channel coding for UCI channel quality information -- see below
% mode 3: Channel coding for UCI channel quality information and HARQ-ACK
% -- not implemented

%% Paths to m and MEX files
% m-files:
addpath('../common functions');

% MEX-files:
% add the path of the OSLD mex files on your machine:
% if you chose to build, but not install: aloe/build/lib/modrep_osld
% if installed: addpath('/usr/local/mex');
addpath('/usr/local/mex');

%% Symbols
TX = 1;
RX = 0;
channel = 5;
format_1a = 0;
format_1b = 1;
format_2 = 2;   % goes with mode 2
format_2a = 3;
format_2b = 4;
format_3 = 5;

%% Simulation parameters
mode = 2;           % Valide modes: 0, 1, 2, and 3
format = format_2;	% Valid formats: 0 for PUCCH format 1a, 1 for PUCCH format 1b, 2 for PUCCH format 2, 3 for PUCCH format 3
nof_bits = 13;       % Between 1 and 4 bits
subframe = 0;

%% Initialize tables
x = generate_binary_table(13); % 8192x13
M = uci_cqi_table;
iM = mod(x*M.',2);     % 8192x20

% Generate random input bits of length nof_bits
input = randi(2,1,nof_bits)==1

if (nof_bits > 13)
    fprintf('Number of input bits %d exceeds maximum size of 13.\n', nof_bits);
    return;
end

%% Transmitter processing chain 
out_coding = pucch_coding(input, mode, M, iM, TX)
out_coding = out_coding==ones(1,length(out_coding));

if (format > 1)
   out_scrambling = am_lte_scrambling(out_coding, {{'subframe',int32(subframe)}, {'channel',int32(channel)}})
end

out_modulation = pucch_modulation(out_scrambling, format, TX)

%% Channel
channel_in = out_modulation;
channel_out = channel_in;

%% Recevier processing chain 
out_demodulation = pucch_modulation(channel_out, format, RX)

if (format > 1)
   out_descrambling = am_lte_scrambling(out_demodulation, {{'hard',int32(1)}, {'subframe',int32(subframe)}, {'channel',int32(channel)}})
end

out_decoding = pucch_coding(out_descrambling, mode, M, iM, RX);
output = x(out_decoding,:)


%% Check if correctly decoded
plot(1:nof_bits, input-output(1:nof_bits), '-+');
axis tight;
title('Decoder input bits minus decoded bits');

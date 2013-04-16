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

% LTE (physical) broadcast channel (PBCH), Tx & Rx DSP

% PBCH carries important PHY information: system bandwidth, number of 
% transmit antennas (implicitly), PHICH configuration and system frame 
% number, ...

% The Master Information Block (MIB) is transmitted using the BCH. It 
% consists of a very limited amount of system information, which is 
% absolutely needed for a UE to be able to read the remaining system 
% information send over the DL-SCH. The MIB contains the following 
% information:
% - DL cell bandwidth, 3 bits: bandwidths measured in # of resource blocks.
% - PHICH configuration of the cell, 3 bits
% - System Frame Number (SFN), 8 bits: 8 MSBs of the SFN, i.e., all bits 
%   except the 2 LSBs of the SFN.
% - Remaining bits, 10 bits, are spare bits (set to zero)

%% Paths to m and MEX files
% m-files:
addpath('../common functions');

% MEX-files:
addpath('/usr/local/mex');
addpath('/home/vuk/DATOS/workspace/lte_ctrl_ratematching'); % Rate Matching (Tx & Rx)
addpath('/home/vuk/DATOS/workspace/lte_crc_scrambling');    % CRC Scrambling/Descrambling


%% Parameters
% Symbolic constants
TX = 0;     % transmission mode (forward processing)
RX = 1;     % reception mode (reverse processing)
QPSK = 1;   % QPSK modulation
FRAME_WISE = 1; % if set, splits the execution of PBCH-Tx and PBCH-Rx into 4 frames (once, per frame) according to the Res. Mapping
INPUT = 1;  % 0: random input bits, 1: master information block (MIB)

% information size in bits (each 4 radio frames)
BITS = 24;  % 24 bits are generated once every 40 ms

% CRC parameters
L = 16;     % applied to the 24 information bits

% Modulation parameters
sigma2 = 1;         % noise variance
LLR_approx = 2;     % 1: exact; 2: approximate LLR
LLR_approx_mex = LLR_approx-1;     % 0: exact; 1: approx. LLR

% Layer mapping and Precoding configuration
style = 0;  % only possible modes: single antenna port (0) or tx diversity (1)
nof_q = 1;  % number of codewords only one codeword for PBCH
v = 2;      % v = 2 or 4 if style = 1
if (style == 0) % single ap
    v = 1;
    p = 0;
else % tx diversity
    if (v == 2)
        p=[0,1];
    else
        p=[0,1,2,3];
    end
end

% CRC scrambling
nof_antenna_ports = v;  % 1, 2, or 4 Tx antenna ports

% Conv. Coding parameters and output
D = BITS+L;     % number of input bits
repetition = 3; % code rate = 1/repetition

% Rate Matching (RM) parameters
turbo = 0;          % convolucional coding (parameter of Matlab model only)
extended_cp = 0;    % if set, indicates extended cyclic prefix
if (extended_cp == 0)
     E = 1920;
else
     E = 1728;
end
 
S = 3*D;        % RM parameter at the Rx

if (FRAME_WISE)
    output = zeros(4,(BITS+L)/4);
    outputx = zeros(4,(BITS+L)/4);
else
    output = zeros(1,(BITS+L));
    outputx = zeros(1,(BITS+L));
end

%% PBCH Tx: Transmitter DSP is done in one frame

% Generate random input bit stream
if (INPUT)
    bw = 6;         % # of resource blocks
    phich_dur = 0;  % 0 indicates normal, 1 extended
    phich_res = 1/6;% possible values: 1/6, 1/2, 1, and 2 
    sfn = 500;      % integer value between 0 and 1023
    input = lte_bcch_bch_msg_pack(bw, phich_dur, phich_res, sfn)==ones(1,24)
else
    input = (randi(2,1,BITS)==1)
end

% CRC attachment
% The matlab model simply adds L zeros
crc_out = [input, zeros(1,L)==ones(1,L)];
%crc_out = am_gen_crc(input, {{'direction',int32(0)}, {'poly',int32(11021)}}); % 0x11021 es el polynomio que toca para PBCH

% CRC Scrambling
scrambled_crc = crc_scrambling(crc_out, 0, 0, 0, 0, nof_antenna_ports, 1);
scrambled_crcx = am_lte_crc_scrambling(crc_out, {{'direction',int32(TX)},{'channel',int32(1)},{'nof_ports',int32(nof_antenna_ports)}});

if (FRAME_WISE)
    frame_size = 10;
    i_top = 4;
    E = E/4;
    S = S/4;
else    % transmit and receive all data at once
    frame_size = 40;
    i_top = 1;
end
    
for i=0:i_top-1
    
    frame = i;
    sample = i*frame_size;
    coder_in = scrambled_crc(i*frame_size+1:(i+1)*frame_size);
    coder_inx = scrambled_crcx(i*frame_size+1:(i+1)*frame_size);
    
    % Coding
    % The model assumes repetition coding, should be Convolutional Coding
    coded_pdcch = repetition_coding(coder_in, repetition);
    coded_pdcchx = repetition_coding(coder_inx, repetition);
    coded_pdcchx = coded_pdcchx==ones(1,length(coded_pdcchx));

    % Rate Matching
    rmatched_pddch = ctrl_ratematching(coded_pdcch, turbo, E);
    rmatched_pddchx = am_lte_ctrl_ratematching(coded_pdcchx, {{'direction',int32(TX)},{'E',int32(E)}});

    % Scrambling
    scrambled_pdcch = pbch_scrambling(rmatched_pddch, frame, sample+1, TX);
    scrambled_pdcchx = am_lte_scrambling(rmatched_pddchx, {{'channel',int32(3)},{'sample',int32(sample)}});

    % Modulation
    modulated_pdcch = qpsk_modulation(scrambled_pdcch);
    modulated_pdcchx = am_gen_modulator(scrambled_pdcchx, {{'modulation',int32(QPSK)}});

    % Layer Mapping (MIMO)
    layered_pdcch = lte_PDSCH_layer_mapper(modulated_pdcch, 0, v, nof_q, style);
    layered_pdcchx = lte_PDSCH_layer_mapper(modulated_pdcchx, 0, v, nof_q, style);

    % Precoding (MIMO)
    precoded_pdcch = lte_PDSCH_precoding(layered_pdcch, p, style);
    precoded_pdcchx = lte_PDSCH_precoding(layered_pdcchx, p, style);

    %% Channel
    % The 24 raw information bits are generated on every forth radio frame
    
    channel_in = precoded_pdcch;
    channel_inx = precoded_pdcchx;

    channel_out = channel_in;
    channel_outx = channel_inx;

    %% PDCCH Rx: Receiver DSP is done in 4 consecutive frames
    
    % Layer Demapping (MIMO)
    unprecoded_pdcch = lte_PDSCH_unprecoding(channel_out, v, style);
    unprecoded_pdcchx = lte_PDSCH_unprecoding(channel_outx, v, style);

    % Un-precoding (MIMO)
    unlayered_pdcch = lte_PDSCH_layer_demapper(unprecoded_pdcch, v, nof_q, style);
    unlayered_pdcchx = lte_PDSCH_layer_demapper(unprecoded_pdcchx, v, nof_q, style);

    % Soft Demodulation
    demodulated_pdcch = soft_demapper(unlayered_pdcch, QPSK, LLR_approx, 0, 1, sigma2, 0);
    demodulated_pdcchx = am_gen_soft_demod(unlayered_pdcchx, {{'soft',int32(LLR_approx_mex)},{'modulation',int32(QPSK)},{'sigma2',sigma2}});

    % Descrambling
    descrambled_pdcch = pbch_scrambling(demodulated_pdcch, i, sample+1, RX);
    descrambled_pdcchx = am_lte_scrambling(demodulated_pdcchx, {{'sample',int32(sample)},{'channel',int32(3)},{'hard',int32(0)}});

    % Rate Matching @Rx
    unrmatched_pdcch = ctrl_unratematching(descrambled_pdcch, turbo, S);
    unrmatched_pdcchx = am_lte_ctrl_ratematching(descrambled_pdcchx, {{'direction',int32(RX)},{'S',int32(S)}});

    % Decoding
    % The model assumes repetition decoding, should be convolutional decoding
    decoded_pdcch = repetition_decoding(unrmatched_pdcch, repetition, 1);
    decoded_pdcchx = repetition_decoding(unrmatched_pdcchx, repetition, 1);
    decoded_pdcchx = decoded_pdcchx==ones(1,length(decoded_pdcchx)); % tranform to logical signal

    processed_information_bits = length(decoded_pdcch)

    % buffer output
    output(i+1,:) = decoded_pdcch;
    outputx(i+1,:) = decoded_pdcchx;

end

output_v = reshape(output',1,BITS+L);
output_vx = reshape(outputx',1,BITS+L)==1;
    
% CRC Descrambling
descrambled_crc = crc_scrambling(output_v, 0, 0, 0, 0, nof_antenna_ports, 1);
descrambled_crcx = am_lte_crc_scrambling(output_vx, {{'direction',int32(RX)},{'channel',int32(1)},{'nof_ports',int32(nof_antenna_ports)}});


% CRC detachment: removes the last L samples
uncrc_out = descrambled_crc(1:length(descrambled_crc)-L);
uncrc_outx = descrambled_crcx(1:length(descrambled_crcx)-L);

% Check difference between TX input and RX output
in_out_diff = uncrc_out - input;
in_out_diffx = uncrc_outx - input;

% Visualize results
X = length(in_out_diff);
XX = length(in_out_diffx);
subplot(2,1,1), plot(1:X, in_out_diff);
title('Input-output difference of Tx-Rx-model based on m files');
axis tight;
subplot(2,1,2), plot(1:XX, in_out_diffx);
title('Input-output difference of Tx-Rx-model based on MEX files');
axis tight;
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
addpath('/home/vuk/DATOS/workspace/lte_bch_demux');         % Time Demultiplexing
addpath('/home/vuk/DATOS/workspace/lte_ctrl_ratematching'); % Rate Matching (Tx & Rx)
addpath('/home/vuk/DATOS/workspace/lte_crc_scrambling');    % CRC Scrambling/Descrambling


%% Parameters
% Symbolic constants
TX = 0;     % transmission mode (forward processing)
RX = 1;     % reception mode (reverse processing)
QPSK = 2;   % QPSK modulation
FRAMES = 1; % number of radio frames to simulate
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
     E = 1920;      %%% data spread over 4 consecutive radio frames
else
     E = 1728;
end
 
S = 3*D;            % RM parameter at the Rx


%% PBCH Tx

for i=1:1
    
    subframe = i;
    
    % this adjusts the begginning of the scrambling sequence, which should
    % be reset every 40 ms (4 radio frames)
    sample = floor(mod(subframe,40)/10)*E;
    sample
    
    if (mod(subframe, 40) == 0)
        % Generate random input bit stream
        if (INPUT)
            bw = 6;         % # of resource blocks
            phich_dur = 0;  % 0 indicates normal, 1 extended
            phich_res = 1/6;% possible values: 1/6, 1/2, 1, and 2 
            sfn = 500;      % integer value between 0 and 1023
            input = lte_bcch_bch_msg_pack(bw, phich_dur, phich_res, sfn)==ones(1,24);
            input_saved = input;
        else
            input = (randi(2,1,BITS)==1);
        end
    else
        input = [];
    end
    
    % CRC attachment
    % The matlab model simply adds L zeros
    if (length(input) > 0)
        crc_out = [input, zeros(1,L)==ones(1,L)]
	else
        crc_out = {};   % no input
    end
    %crc_out = am_gen_crc(input, {{'direction',int32(0)}, {'poly',int32(11021)}}); % 0x11021 es el polynomio que toca para PBCH

    % CRC Scrambling (return 0 - done)
    scrambled_crcx = am_lte_crc_scrambling(crc_out, {{'direction',int32(TX)},{'channel',int32(1)},{'nof_ports',int32(nof_antenna_ports)}});
    
    % Coding (return 0 - done)
    % The model assumes repetition coding, should be Convolutional Coding
    coded_pdcchx = repetition_coding(scrambled_crcx, repetition);
	if (length(coded_pdcchx)>0)
        coded_pdcchx = coded_pdcchx==ones(1,length(coded_pdcchx));
    end

    % Rate Matching (return 0 - done)
    %rmatched_pddch = ctrl_ratematching(coded_pdcch, turbo, E);
    rmatched_pddchx = am_lte_ctrl_ratematching(coded_pdcchx, {{'direction',int32(TX)},{'E',int32(E)}});
    %if (length(rmatched_pddch)>0)
    %    rmatched_pddch = rmatched_pddch==ones(1,length(rmatched_pddch));
    %end
    
    % Isma, no sé como hacer que devuelva un cell arrary de 0x0, porque la
    % salida de un modulo siendo un logical array de 1x0 (1 interfaces x 0 elements) 
    % el siguiente modulo no lo acepta como entrada
    in_demux_l=length(rmatched_pddchx)
    if (in_demux_l == 0)
        rmatched_pddchx = {};
    end
    
    % Demultiplexing: delivers 1/4th of the data each radio frame (every
    % 10th subframe
    %demux_out = am_lte_bch_demux(rmatched_pddch);
    demux_outx = am_lte_bch_demux(rmatched_pddchx)
    
    if (length(demux_outx) == 0)
        demux_outx = {};
    end
    
    % Scrambling (return 0 - done)
    %scrambled_pdcch = pbch_scrambling(rmatched_pddch, sample+1, TX);
    scrambled_pdcchx = am_lte_scrambling(demux_outx, {{'channel',int32(3)},{'sample',int32(sample)}});

    if (length(scrambled_pdcchx) == 0)
        scrambled_pdcchx = {};
    end
    
    % Modulation (return 0 - done)
    %modulated_pdcch = qpsk_modulation(scrambled_pdcch);
    modulated_pdcchx = am_gen_modulator(scrambled_pdcchx, {{'modulation',int32(QPSK)}});

    % Layer Mapping (MIMO) (return 0 - done)
    %layered_pdcch = lte_PDSCH_layer_mapper(modulated_pdcch, 0, v, nof_q, style);
    %layered_pdcchx = lte_PDSCH_layer_mapper(modulated_pdcchx, 0, v, nof_q, style);

    % Precoding (MIMO) (return 0 - done)
    %precoded_pdcch = lte_PDSCH_precoding(layered_pdcch, p, style);
   % precoded_pdcchx = lte_PDSCH_precoding(layered_pdcchx, p, style);

    %% Channel
    
    %channel_in = precoded_pdcch;
    channel_inx = modulated_pdcchx;

    %channel_out = channel_in;
    channel_outx = channel_inx;

    %% PDCCH Rx: Receiver DSP is done in 4 consecutive frames
    
    % Layer Demapping (MIMO) (return 0 - done)
    %unprecoded_pdcch = lte_PDSCH_unprecoding(channel_out, v, style);
    %unprecoded_pdcchx = lte_PDSCH_unprecoding(channel_outx, v, style);

    % Un-precoding (MIMO) (return 0 - done)
    %unlayered_pdcch = lte_PDSCH_layer_demapper(unprecoded_pdcch, v, nof_q, style);
   % unlayered_pdcchx = lte_PDSCH_layer_demapper(unprecoded_pdcchx, v, nof_q, style);

    % Soft Demodulation (return 0 - done)
    %demodulated_pdcch = soft_demapper(unlayered_pdcch, QPSK, LLR_approx, 0, 1, sigma2, 0);
    demodulated_pdcchx = am_gen_soft_demod(channel_outx, {{'soft',int32(LLR_approx_mex)},{'modulation',int32(QPSK)},{'sigma2',sigma2}});

    binary=demodulated_pdcchx>0;
    if (length(demodulated_pdcchx)==0)
        demodulated_pdcchx = {};
    end
        
    % Descrambling (return 0 - done)
    %descrambled_pdcch = pbch_scrambling(demodulated_pdcch, sample+1, RX);
    descrambled_pdcchx = am_lte_scrambling(demodulated_pdcchx, {{'sample',int32(sample)},{'channel',int32(3)},{'hard',int32(0)}});

    if (length(descrambled_pdcchx)==0)
        descrambled_pdcchx = {};
    end
    
    % Rate Matching @Rx (return 0 - done)
    %unrmatched_pdcch = ctrl_unratematching(descrambled_pdcch, turbo, S);
    unrmatched_pdcchx = am_lte_ctrl_ratematching(descrambled_pdcchx, {{'direction',int32(RX)},{'S',int32(S)}});

    % Decoding (return 0 - done)
    % The model assumes repetition decoding, should be convolutional decoding
    %decoded_pdcch = repetition_decoding(unrmatched_pdcch, repetition, 1);
    decoded_pdcchx = repetition_decoding(unrmatched_pdcchx, repetition, 1);
    if (length(decoded_pdcchx)>0)
        decoded_pdcchx = decoded_pdcchx==ones(1,length(decoded_pdcchx)); % tranform to logical signal
    end

    processed_information_bits = length(decoded_pdcchx)

    % CRC Descrambling (return 0 - done)
    %descrambled_crc = crc_scrambling(decoded_pdcch, 0, 0, 0, 0, nof_antenna_ports, 1);
    descrambled_crcx = am_lte_crc_scrambling(decoded_pdcchx, {{'direction',int32(RX)},{'channel',int32(1)},{'nof_ports',int32(nof_antenna_ports)}});

    % CRC detachment: removes the last L samples
    %uncrc_out = descrambled_crc(1:length(descrambled_crc)-L);
    uncrc_outx = descrambled_crcx(1:length(descrambled_crcx)-L);

    if (length(uncrc_outx) > 0)
        % Check difference between TX input and RX output
        %in_out_diff = uncrc_out - input;
        in_out_diffx = uncrc_outx - input_saved;

        % Visualize results
        %X = length(in_out_diff);
        XX = length(in_out_diffx);
%         subplot(2,1,1), plot(1:X, in_out_diff);
%         title('Input-output difference of Tx-Rx-model based on m files');
%         axis tight;
        figure;
        plot(1:XX, in_out_diffx);
        title('Input-output difference of Tx-Rx-model based on MEX files');
        axis tight;
    end
    
end

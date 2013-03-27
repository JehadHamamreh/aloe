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


% LTE physical downlink control channel (PDCCH), Tx & Rx, symbol level

% Control signaling is required to support the transmission of the DL and 
% UL transport channels (DL-SCH and UL-SCH). Control information for one or 
% multiple UEs is contained in a Downlink scheduling Control Information 
% (DCI) message and is transmitted through the PDCCH. DCI messages contain 
% information including:
% - DL-SCH resource allocation (the set of resource blocks containing the 
%   DL-SCH) and modulation and coding scheme. This information allows the 
%   UE to decode the DL-SCH.
% - Transmit Power Control (TPC) commands for the PUCCH and UL-SCH adapt 
%   the transmit power of the UE to save power.
% - Hybrid-Automatic Repeat Request (HARQ) information including the 
%   process number and redundancy version for error correction
% - MIMO precoding information


%% Paths to m and MEX files
    % m-files:
addpath('layer mapper');          % contains layer mapper/demapper and precoder/un-precoder
addpath('../PCFICH/CFICH');                 % contains QPSK modem
%addpath('modulation demapping');  % contains Soft Demapper
addpath('../')
% MEX-files:
addpath('/usr/local/mex');

%addpath('/home/vuk/DATOS/workspace/lte_scrambling');        % Scrambling
%addpath('/home/vuk/DATOS/workspace/lte_descrambling');      % Descrambling of soft bits

%addpath('/home/vuk/DATOS/workspace/lte_modem');             % Modulator
%addpath('/home/vuk/DATOS/workspace/gen_soft_demod')         % Soft Demodulator


%% Parameters
% CRC parameters
L = 16;

% CRC scrambling
antenna_selection = 0;  % 0: UE transmit antenna selection is not configured or applicable, >0 if configured and applicable
dci_format = 0; 
rnti = 4327;    
ue_port=1;      % 0 or 1

% Scrambling parameters
channel = 2;        % 2 indicates PDCCH
q = 0;              % must be 0 or not specified
ns = 0;             % slot index
cell_gr =   101;    % Physical-layer cell-identity group, range: integer in [0, 167], default: 0
cell_sec =  2;      % Physical-layer identity within the physical-layer identity group, range: integer in [0, 2], default: 0
direct = 0;         % implementation parameter, set if processing short bit-sequences

% Modulation parameters
QPSK = 1;           % symbolc symbol for qpsk modulation
sigma2 = 1;         % noise variance
LLR_approx = 2;     % 1: exact; 2: approximate LLR
LLR_approx_mex = LLR_approx-1;     % 0: exact; 1: approx. LLR

% Layer mapping and Precoding configuration
style = 1;  % only possible modes: single antenna port (0) or tx diversity (1)
nof_q = 1;  % number of codewords only one codeword for PDCCH
v = 2;      % v = 2 or 4 if style = 1
if (style == 0)
    v = 1;
    p = 0;  % single ap
else
    if (v == 2)
        p=[0,1];    % tx diversity
    else
        p=[0,1,2,3];% tx diversity
    end
end

% Conv. Coding parameters and output
D = 180; % # bits of each conv. enconder stream (total of 3*M bits per PDCCH and subframe)
repetition = 3; % code rate = 1/repetition
%d = [(randi(2,1,D)==1); (randi(2,1,D)==1); (randi(2,1,D)==1)];   % the three output bits streams of the conv. enconder

% Rate Matching (RM) parameters
turbo = 0;      % convolucional coding
pdcch_format = 2;     % valid formats: 0, 1, or 2
switch (pdcch_format)
    case 0
        E = 144;
    case 1
        E = 288;
    case 2
        E = 576;        % RM parameter at the transmitter: # RM-Tx output bits. Only three valid formats: 144 bits),
    otherwise,
        fprintf('\nError: Incorrect PDCCH format %d.\n', i);
        return;
end
S = 3*(D+L);   % RM parameter at the transmitter: # RM-Rx output bits

% Multiplexing parameters
n = 1;          % # of PDCCHs transmitted in the subframe
Nreg = E/8;     % Number of available resource-element groups for PDCCH, should at be least larger than sum(M)/8 (8 bits per resource element group)


%% PDCCH Tx

%The physical downlink control channel carries scheduling assignments and 
% other control information. 

% A PDCCH is transmitted on one or an aggregation of several consecutive 
% control channel elements (CCEs). A CCE is a group of nine consecutive 
% resource-element groups (REGs). The number of CCEs used to carry a PDCCH 
% is controlled by the PDCCH format. 
% A PDCCH format of 0, 1, 2, or 3 corresponds to 1, 2, 4 or 8 consecutive 
% CCEs being allocated to one PDCCH.

% Generate random input bit stream
input = (randi(2,1,D)==1);

% CRC attachment
% The matlab model simply adds L zeros
crc_out = [input, zeros(1,L)];
%crc_out = am_gen_crc(input, {{'direction',int32(0)}, {'poly',int32(11021)}}); % 0x11021 es el polynomio que toca para PDCCH

% CRC Scrambling
scrambled_crc = crc_scrambling(crc_out, antenna_selection, dci_format, rnti, ue_port);
scrambled_crcx = crc_scrambling(crc_out, antenna_selection, dci_format, rnti, ue_port); % SUBSTITUTE WITH MEX when ready!
%unscrambled_crcx = am_lte_crc_scrambling(uncoded_pdcchx, {{'direction',int32(0)}, {'selection', int32(antenna_selection)}, {'format',int32(dci_format)}, {'rnti',int32(rnti)}, {'port',int32(ue_port}});

% Coding
% The model assumes repetition coding, should be Convolutional Coding
coded_pdcch = repetition_coding(scrambled_crc, repetition);
coded_pdcchx = repetition_coding(scrambled_crcx, repetition);
coded_pdcchx = coded_pdcchx == ones(1,length(coded_pdcch)); % tranform to logical signal

% Rate Matching
rmatched_pddch = pdcch_rm(coded_pdcch, turbo, E);
rmatched_pddchx = am_lte_ctrl_ratematching(coded_pdcchx', {{'direction',int32(0)},{'E',int32(E)}});

% PDCCH multiplexing: joins several PDCCHs
% ONLY MATLAB MODEL AVAILABLE
multiplexed_pdcch = pdcch_mux(rmatched_pddch, n, Nreg);
multiplexed_pdcchx = pdcch_mux(rmatched_pddchx, n, Nreg);
multiplexed_pdcchx = multiplexed_pdcchx == ones(1,length(multiplexed_pdcchx)); % tranform to logical signal

% Scrambling
scrambled_pdcch = dci_scrambling(multiplexed_pdcch, cell_gr, cell_sec, ns, 0);
scrambled_pdcchx = am_lte_scrambling(multiplexed_pdcchx', {{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'channel',int32(channel)},{'direct',int32(direct)}});

% Modulation
modulated_pdcch = qpsk_modulation(scrambled_pdcch);
modulated_pdcchx = am_gen_modulator(scrambled_pdcchx', {{'modulation',int32(2)}});

% Layer Mapping (MIMO)
layered_pdcch = lte_PDSCH_layer_mapper(modulated_pdcch, 0, v, nof_q, style);
layered_pdcchx = lte_PDSCH_layer_mapper(modulated_pdcchx, 0, v, nof_q, style);

% Precoding (MIMO)
precoded_pdcch = lte_PDSCH_precoding(layered_pdcch, p, style);
precoded_pdcchx = lte_PDSCH_precoding(layered_pdcchx, p, style);

%% Channel
channel_out = precoded_pdcch;
channel_outx = precoded_pdcchx;

%% PDCCH Rx
% Layer Demapping (MIMO)
unprecoded_pdcch = lte_PDSCH_unprecoding(channel_out, v, style);
unprecoded_pdcchx = lte_PDSCH_unprecoding(channel_outx, v, style);

% Un-precoding (MIMO)
unlayered_pdcch = lte_PDSCH_layer_demapper(unprecoded_pdcch, v, nof_q, style);
unlayered_pdcchx = lte_PDSCH_layer_demapper(unprecoded_pdcchx, v, nof_q, style);

% Soft Demodulation
demodulated_pdcch = soft_demapper(unlayered_pdcch, QPSK, LLR_approx, sigma2);
demodulated_pdcchx = am_gen_soft_demod(unlayered_pdcchx', {{'soft',int32(LLR_approx_mex)},{'modulation',int32(2)},{'sigma2',sigma2}});

u=load('unlayered.mat');
u=u.unlayered_pdcch;
x=u';
out_mex=am_gen_soft_demod(unlayered_pdcch', {{'soft',int32(1)},{'modulation',int32(2)},{'sigma2',1.5}}); % soft = 0: exact; soft = 1: approx. LLR
out_matlab=soft_demapper(unlayered_pdcch, 1, 2, 1.5); % 2 means approx. LLR, 1 exact

subplot(1,1,1)
N=10;
plot(1:bits_per_symbol*N,out_matlab(1:bits_per_symbol*N),'b-',1:bits_per_symbol*N,out_mex(1:bits_per_symbol*N),'r--');

% PDCCH Demultiplexing?
% missing

% Descrambling
descrambled_pdcch = dci_scrambling(demodulated_pdcch, cell_gr, cell_sec, ns, 1);
descrambled_pdcchx = am_lte_descrambling(demodulated_pdcchx', {{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'channel',int32(channel)},{'direct',int32(direct)}});

% Rate Matching @Rx
unrmatched_pdcch = pdcch_unrm(descrambled_pdcch, turbo, S);
unrmatched_pdcchx = am_lte_ctrl_ratematching(descrambled_pdcchx', {{'direction',int32(1)},{'S',int32(S)}});

% Decoding
% The model assumes repetition decoding, should be convolutional decoding
uncoded_pdcch = repetition_decoding(unrmatched_pdcch, repetition, 1);
uncoded_pdcchx = repetition_decoding(unrmatched_pdcchx, repetition, 1);

% CRC Descrambling
unscrambled_crc = crc_scrambling(uncoded_pdcch, antenna_selection, dci_format, rnti, ue_port);
unscrambled_crcx = crc_scrambling(uncoded_pdcchx, antenna_selection, dci_format, rnti, ue_port); %SUBSTITUTE WITH MEX when ready!
%unscrambled_crcx = am_lte_crc_scrambling(uncoded_pdcchx, {{'direction',int32(1)}, {'selection', int32(antenna_selection)}, {'format',int32(dci_format)}, {'rnti',int32(rnti)}, {'ue_port',int32(ue_port}});

% CRC detachment: removes the last L samples
uncrc_out = unscrambled_crc(1:length(unscrambled_crc)-L);
uncrc_outx = unscrambled_crcx(1:length(unscrambled_crcx)-L);

% Check difference between TX input and RX output
in_out_diff = uncrc_out - input;
in_out_diffx = uncrc_outx - input;

% Visualize results
% X = length(uncrc_out);
% XX = length(uncrc_outx);
% subplot(2,1,1), plot(1:X, in_out_diff);
% title('Input-output difference of Tx-Rx-model based on m files');
% subplot(2,1,2), plot(1:XX, in_out_diffx);
% title('Input-output difference of Tx-Rx-model based on MEX files');

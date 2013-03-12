% /* 
%  * Copyright (c) 2012-2013, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
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


% Control Format Indication Channel (PCFICH)
% transports ths Control Format Indicator (CFI) value. 
% The CFI defines the time span, in OFDM symbols, of the PDCCH transmission 
% for a particular downlink subframe. That is, it specifies how many OFDM 
% symbols are used to transmit the control channels so the receiver knows 
% where to find control information.



%% Parameters
% Inputs:
% cfi           - Control Format Indicator (CFI): 1, 2, or 3
%
%
% Outputs:
% out           
%

addpath('../layer mapper');
addpath('../scrambling');
addpath('/usr/local/mex');

x = rand(1); % Control Format Indicator (CFI): 1, 2, or 3
    cfi = 1;

channel = 1;        % 1 indicates PCFICH
q = 0;              % don't care
ns = 7;             % slot index
cell_gr =   101;    % Physical-layer cell-identity group, range: integer in [0, 167], default: 0
cell_sec =  2;      % Physical-layer identity within the physical-layer identity group, range: integer in [0, 2], default: 0
nRNTI =     65530;  % don't care, range: integer in [0, 65535], default: 0


% Layer mapping and Precoding configuration
style = 1;  % only possible modes: single antenna port (0) or tx diversity (1)
nof_q = 1;      % number of codewords only one codeword for PCFICH
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

% PCFICH Tx
coded_cfi = cfi_coding(cfi)
%coded_cfix = am_lte_cfi_coding(int32(cfi)); % cannot deal with integer inputs or outputs

scrambled_cfi = cfi_scrambling(coded_cfi, cell_gr, cell_sec, ns)
scrambled_cfix = am_lte_scrambling(coded_cfi', {{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'channel',int32(channel)}});

modulated_cfi = qpsk_modulation(scrambled_cfi)
modulated_cfix = am_gen_modulator(scrambled_cfix', {{'modulation',int32(2)}});

layered_cfi = lte_PDSCH_layer_mapper(modulated_cfi, 0, v, nof_q, style)
precoded_cfi = lte_PDSCH_precoding(layered_cfi, p, style)

% Channel
channel_out = precoded_cfi;

% PCFICH Rx
unprecoded_cfi = lte_PDSCH_unprecoding(channel_out, v, style)
unlayered_cfi = lte_PDSCH_layer_demapper(unprecoded_cfi, v, nof_q, style)

demodulated_cfi = qpsk_demodulation(unlayered_cfi)
demodulated_cfix = am_gen_hard_demod(unlayered_cfi', {{'modulation',int32(2)}});

descrambled_cfi = cfi_scrambling(demodulated_cfi, cell_gr, cell_sec, ns)
descrambled_cfix = am_lte_hard_descrambling(demodulated_cfix', {{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'channel',int32(channel)},{'direct',int32(1)}});

uncoded_cfi = cfi_decoding(descrambled_cfi)
%uncoded_cfix = am_lte_cfi_decoding(descrambled_cfix); % cannot deal with integer inputs or outputs

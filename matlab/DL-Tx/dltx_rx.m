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

% LTE Downlink Transmitter and Receiver Processing Chain: Symbol Level
%
% Mix of Matlab m-functions and MEX files
%

clear all;
close all;

%% Paths to m and MEX files
% m-files:
addpath('../common functions');

% MEX-files:
% add the path of the OSLD mex files on your machine:
% if you chose to build, but not install: aloe/build/lib/modrep_osld
% if installed: addpath('/usr/local/mex');
addpath('mex');

%% Symbols
% transmission or reception
TX = 1;
RX = 0;

% modulation types:
BPSK = 0;
QPSK = 1;
QAM16 = 2;
QAM64 = 3;

% MIMO style:
single_ap = 0;
tx_div = 1;
sp_mux = 2;

% FFT points/:
fft128 = 128;   % Max. Tr-Bw = 6*12*15 kHz = 1.08 MHz (Ch_Bw = 1.4 MHz)
fft256 = 256;   % Max. Tr-Bw = 15*12*15 kHz = 2.7 MHz (Ch_Bw = 3 MHz)
fft512 = 512;   % Max. Tr-Bw = 25*12*15 kHz = 4.5 MHz (Ch_Bw = 5 MHz)
fft1024 = 1024; % Max. Tr-Bw = 50*12*15 kHz = 9 MHz (Ch_Bw = 10 MHz)
fft1536 = 1536; % Max. Tr-Bw = 75*12*15 kHz = 13.5 MHz (Ch_Bw = 15 MHz)
fft2048 = 2048; % Max. Tr-Bw = 100*12*15 kHz = 18 MHz (Ch_Bw = 20 MHz)


%% Simulation Parameters
% 1. Simulation slots (20 slots = 10 subframes = 1 radio frame = 10 ms):
SLOTS = 20;
SUBFRAMES = 10;

%**** EDIT HERE ****%
iMAX = SUBFRAMES;
UL = 1;
fft_points = fft1024;
ofdm_symbols_per_slot = 7;      % 6 or 7
%*******************%

% 2. Transmission parameters defining the WF transmission mode
% 2.1 Modulation:
modulation = QPSK;
sigma2 = 2;         % noise variance
soft = 1;           % 0: exact; 1: approx. LLR (use soft+1 in soft_demapper.m or 0 for hard demod)
ZERO = 0;           % hard demodulation mapping '0'
ONE = 1;            % hard demodulation mapping '1'

% 2.2 MIMO:
mode = 0;

% MIMO mode configuration:
if (mode == 0)
    q = 1; v = 1; p = 0; style = single_ap;
elseif (mode == 1)
    q = 1;  % Number of transport blocks (code blocks)
    v = 2;  % Number of layers (see valid combination @lte_PDSCH_layer.m)
    p = [0,1];  % antenna port vector (see lte_PDSCH_precoding.m)
    style = tx_div; % MIMO mode
elseif (mode == 2)
    q = 1;  % Number of transport blocks (code blocks)
    v = 4;  % Number of layers (see valid combination @lte_PDSCH_layer.m)
    p = [0,1,2,3];  % antenna port vector (see lte_PDSCH_precoding.m)
    style = tx_div; % MIMO mode    
else
    q = 1;  % Number of transport blocks (code blocks)
    v = 4;  % Number of layers (see valid combination @lte_PDSCH_layer.m)
    p = [0,1,2,3];  % antenna port vector (see lte_PDSCH_precoding.m)
    style = sp_mux; % MIMO mode    
end
ap = length(p);

ofdm_symbols_per_subframe = 2*ofdm_symbols_per_slot; % 12 or 14
if (iMAX == SLOTS)
    ofdm_symbols = ofdm_symbols_per_slot;
else
    ofdm_symbols = ofdm_symbols_per_subframe;
end

if (ofdm_symbols_per_slot == 7)
    normal = 1; % normal CP
else
    normal = 0; % extended CP
end

[sc, zero_points, cp_samples, cp_samples_first, bits_per_symbol, fs] = configure_params(fft_points, normal, modulation);

if (mod(zero_points,2) ~= 0)
    fprintf('\nWarning: Unused subcarriers not integer divisible by 2. Be cautious with the resource mapping.');
end        

%% Function parameters:
% ***** Tx/Rx *****
% modulator_params:     'modulation' = modulation
%
% layer_mapping_params: 
%
% precoding_params:     
%
%
% padding_params:       'data_type' = int32(0);
%                       'pre_padding' = int32(prep) [prep: #zeros padded at beginning];
%                       'post_padding' = int32(postp) [postp: #zeros padded at end]
%                       'nof_packets' = int32(ofdm_symbols_per_subframe).
%
% ifft_parms:           'direction' = int32(B) [B = 0 (forward=fft), 1 (backward=ifft)];
%                       'mirror' = int32(Y) [Y = 0 (no mirror), 1 (mirror)];
%                       'normalize' = int32(Y) [Y = 0 (don't normalize), 1 (normalize)];
%                       'dft_size' = int32(fft_points).
%
% cyclic_params:        'ofdm_symbol_sz' = int32(fft_points)
%                       'cyclic_prefix_sz' = int32(cp_samples)
%                       'first_cyclic_prefix_sz',int32(cp_samples_first)
%

modulator_params={{'modulation',int32(modulation)}};

padding_params={{'data_type',int32(0)},{'pre_padding',int32(zero_points/2)},{'post_padding',int32(zero_points/2)},{'nof_packets',int32(ofdm_symbols_per_subframe)}};

% Transform Precoder (Tx, Rx)
% The fftw3 library computes an unnormalized transform/inverse transform.
% Use {'normalize',int32(1)} to normalize with 1/sqrt(dft_size)
dft_params={{'direction',int32(0)},{'normalize',int32(1)},{'mirror',int32(0)},{'dft_size',int32(sc)}}; % Transform Precoder (TX)
idft_params={{'direction',int32(1)},{'normalize',int32(1)},{'mirror',int32(0)},{'dft_size',int32(sc)}};% Transform Precoder (RX)

% OFDM modulator (Tx) & demodulator (Rx)
ifft_dl_params={{'direction',int32(1)},{'dc_offset',int32(1)},{'mirror',int32(1)},{'normalize',int32(1)},{'dft_size',int32(fft_points)}};%,{'psd',int32(0)},{'out_db',int32(0)}};
fft_dl_params={{'direction',int32(0)},{'dc_offset',int32(1)},{'mirror',int32(2)},{'normalize',int32(1)},{'dft_size',int32(fft_points)}};%,{'psd',int32(0)},{'out_db',int32(0)}};

% SC-FDMA modulator (Tx) & demodulator (Rx)
ifft_ul_params={{'direction',int32(1)},{'mirror',int32(1)},{'normalize',int32(1)},{'dft_size',int32(fft_points)},{'df',7500},{'fs',fs}};
fft_ul_params ={{'direction',int32(0)},{'mirror',int32(2)},{'normalize',int32(1)},{'dft_size',int32(fft_points)},{'df',-7500},{'fs',fs}};

cyclic_params={{'ofdm_symbol_sz',int32(fft_points)},{'cyclic_prefix_sz',int32(cp_samples)},{'first_cyclic_prefix_sz',int32(cp_samples_first)}};

%% DSP
output=[];  % Tx antenna output (channel input)
input=[];   % Rx antenna input (channel output)

count = 0;
for i=0:iMAX-1

    if (iMAX == SLOTS)
        subframe = floor(i/2);
    else
        subframe = i;
    end
    
    % Generate random input bis
    in_bits = randi(2,1,(ofdm_symbols*sc)*bits_per_symbol)==1;     % should be integer divisible by 'ofdm_symbols'
    %length(in_bits)
    %in_bits = randi(2,308,1)==1;
    
    %% Symbol level
    % ********** TRANSMITTER **********
    % Scrambling
    out_scrambling = am_lte_scrambling(in_bits, {{'subframe',int32(subframe)}});

    % Modulation
    out_modulator = am_gen_modulator(out_scrambling, modulator_params);
    
    % Layer Mapping
    out_layer = lte_PDSCH_layer_mapper(out_modulator, 0, v, q, style);
    
    if (UL)
        for j=1:v   % v Layers
            % 12 or 14 OFDM Symbols per subframe
            for k=0:(ofdm_symbols-1)
            % Transform Precoding, Tx (UL only), executes only 12 times per
            % subframe? The remaining 2 OFDM symbols are fed with control data?
                out_dft(j,k*sc+1:(k+1)*sc) = 1/sqrt(sc)*fft(out_layer(j,k*sc+1:(k+1)*sc));
            end
            out_dft(j,:) = am_gen_dft(out_layer(j,:), dft_params);
        end
    else
        out_dft = out_layer;
    end
    
    % Precoding
    out_precoding = lte_PDSCH_precoding(out_dft, p, style);

    % ap antenna ports
    for l=1:ap
        % OFDM/SC-FDMA Modulator: Includes padding of zeros (unused subcarriers)
        if (UL)
            % SC-FDMA modem is like OFDM modem, except that DC is not set to 0
            
            count = 0;  % reinitiate sample counter for frequency shift
            
            % IFFT scales the output with 1/fft_points. This is not specified by 3GPP,
            % but is needed when assuming ideal channels and not applying
            % gain control at the receiver
            %out_ifft(l,:) = sc_fdma_modem(out_precoding(l,:), ofdm_symbols, fft_points, sc, TX, count);
            
            out_padding = am_gen_padding(out_precoding(l,:), padding_params);
            out_ifft(l,:) = am_gen_dft(out_padding, ifft_ul_params);
        else
            %out_ifft(l,:) = ofdm_modem(out_precoding(l,:), ofdm_symbols, fft_points, sc, TX);
            
            out_padding = am_gen_padding(out_precoding(l,:), padding_params);
            out_ifft(l,:) = am_gen_dft(out_padding, ifft_dl_params);
        end
        
        %length(out_ifft(l,:))
        % Cyclic prefix
        out_cyclic(l,:) = am_gen_cyclic(out_ifft(l,:), cyclic_params);
        l_out = length(out_cyclic(l,:));
    end
    
    output = out_cyclic;
    
%    output_acc(:,i*l_out+1:(i+1)*l_out) = output;
    
    % Plot spectrum @Tx antennas
    %close all;
    for l=1:ap
        [Pxx,W] = pwelch(output(l,:),length(out_ifft(l,:))/ofdm_symbols,[],4*fft_points);
        figure;
        %plot(10*log10(pwelch(output(l,:))));
        plot(W,10*log10(Pxx));
        xlabel('\Omega/F_S');
        ylabel('Power/Frequency [dB/Hz]');
        %axis([0 2*pi -60 -25]);
        axis tight;
    end
    
    
    % Channel
    input = output; % input: received signal
 
    
    % ********** RECEIVER **********

    % ap antenna ports
    for l=1:ap
        % Cyclic Prefix Detachment
        %out_uncyclic(l,:) = cyclic_detach(input(l,:), ofdm_symbols, fft_points, cp_samples, cp_samples_first); % m-function
        out_uncyclic(l,:) = am_gen_remcyclic(input(l,:), cyclic_params);
    end
    for l=1:ap
        % OFDM/SC-FDMA Demodulator: Includes unpadding of zeros (unused subcarriers)
       	if (UL)
            
            count = 0;  % reinitiate sample counter for frequency shift
            
        	% SC-FDMA modem is like OFDM modem, except that DC is not set
            % to 0 and spectrum centered between subcarrier index sc/2-1 and
            % sc/2
            %out_fft(l,:) = sc_fdma_modem(out_uncyclic(l,:), ofdm_symbols, fft_points, sc, RX, count);
            
            out_fft_aux = am_gen_dft(out_uncyclic(l,:), fft_ul_params);
            out_fft(l,:) = unpadding(out_fft_aux, fft_points, sc);
        else
        	%out_fft(l,:) = ofdm_modem(out_uncyclic(l,:), ofdm_symbols, fft_points, sc, RX);
            
            out_fft_aux = am_gen_dft(out_uncyclic(l,:), fft_dl_params);
            out_fft(l,:) = unpadding(out_fft_aux, fft_points, sc);
        end
	end
    
    % Un-precoding
    out_unprecoding = lte_PDSCH_unprecoding(out_fft, v, style);
    
    if (UL)
        for j=1:v   % v Layers
            for k=0:(ofdm_symbols-1) 
            % Transform Precoding, Rx (UL only)
                out_idft(j,k*sc+1:(k+1)*sc) = sc/sqrt(sc)*ifft(out_unprecoding(j,k*sc+1:(k+1)*sc));
                % Important to scale with sc/sqrt(sc), because the output
                % of the Matlab function ifft is already scaled with 1/sc (like a normal IDFT). 
                % The fftw3 library, on the other hand, computes an unnormalized transform/inverse transform. 
                % Applying the forward and then the backward transform
                % without explicit scaling will multiply the input by n.
            end
            out_idft(j,:) = am_gen_dft(out_unprecoding(j,:), idft_params);
        end
    else
    	out_idft = out_unprecoding;
    end
    
    % Layer Demapping
    out_unlayer = lte_PDSCH_layer_demapper(out_idft, v, q, style);
    
    % Hard Demodulation (Apply soft demodulation when including Turbo encoder/decoder)
    %out_demodulator = soft_demapper(out_unlayer, modulation, 0, ZERO, ONE, sigma2, 0); % thrid parameter: 0 means hard. Use 'soft+1' for LLR and approx. LLR 
    %out_demodulator = (out_demodulator == ones(1,length(out_demodulator)));
	out_demodulator = am_gen_hard_demod(out_unlayer, {{'modulation',int32(modulation)}});
	%out_demodulator = am_gen_soft_demod(out_unlayer, {{'soft',int32(soft)},{'modulation',int32(modulation)},{'sigma2',sigma2}});
    
    % Descrambling (hard descrmabling: hard = 1. Choose hard = 0 when including Turbo encoder/decoder)
    out_descrambling = am_lte_scrambling(out_demodulator, {{'hard',int32(1)},{'subframe',int32(subframe)}});
    
end


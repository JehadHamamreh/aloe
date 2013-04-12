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


% Inputs:      
% in            - Complex valued modulation symbols, 1D vector
%               of N elements (N = 72, 180, 300, 600, 900, 1200)
%
% points        - Number of (fft/ifft) points: 128, 256, 512, 1024, 1536,
%               2048
%
% direction     - 0: fft (RX), 1: ifft (TX)
%
% Outputs:     
% out           - 1D vector of 'points' complex-valued samples
%               

% Spec:         3GPP TS 36.211 section 5.6 v10.5.0
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:1.3.2013
% 

% OFDM modulator


function out = sc_fdma_modem(in, ofdm_symbols_per_subframe, points, sc, direction, count)

    %% Parameters
    df = 15000;         % df = 15 kHz subcarrier spacing
    f0 = 0.5*df;        % Frequency shift by f0.
    M = sc;             % TX: input data lenght per OFDM Symbol, RX: output data lenght per OFDM Symbol
    M_half = M/2;       % input data length half
    N = points;         % fft/ifft-points
    N_half = points/2;  % fft/ifft-points half
    upper_index = N-M_half;
    
    input = zeros(N,1); % TX: ifft input, RX: fft input
    if (direction)  % TX
        out = zeros(1,ofdm_symbols_per_subframe*N);
    else
        out = zeros(1,ofdm_symbols_per_subframe*M);
    end
    
    % Obtain frequency shift vector
    %shift = frequency_shift(points, ofdm_symbols_per_subframe, f0, count, direction);
    shift = frequency_shift(points, 1, f0, count, direction);
%    shift = ones(1,2048);
%     figure;
%     plot(abs(fft(shift,4*points)));
%     axis tight;
    
    for k=0:(ofdm_symbols_per_subframe-1)
    
        if (direction)
            
            input_symbols = in(k*M+1:(k+1)*M);
            
            %% set fft/ifft input points to data samples and pad zeros
        
            % upper half of the input data symbols assigned to M_half consecutive fft/ifft points
            for i=0:(M_half-1)
                input(i+1) = input_symbols(M_half+i+1);
            end
    
            % zero-padding: nof_zeros_half zeros
            for i=M_half:(upper_index-1)
                input(i+1) = 0;
            end
    
            % lower half of the input daya symbols assigned to M_half consecutive fft/ifft points
            for i=upper_index:(N-1)
                input(i+1) = input_symbols(i+1-upper_index);
            end
            
        else
            input = in(k*N+1:(k+1)*N);
        end
     
        %% fft or ifft
        if (direction)
            %output = ifft(input).'.*shift(k*N+1:(k+1)*N);   % ifft (TX)
            output = ifft(input).'.*shift;   % ifft (TX), reinitialized sample counter for frequency shift
            out(k*N+1:(k+1)*N) = output;
        else
            %output = fft(input.*shift(k*N+1:(k+1)*N));      % fft (RX)
            output = fft(input.*shift);      % fft (RX), reinitialized sample counter for frequency shift
            out(k*M+1:(k+1)*M) = [output(upper_index+1:N), output(1:M_half)]; % Unpadding
        end
        
    end 
    
%     plot(abs(output));
%     figure;
%     plot(abs(fft(output)));
    
end


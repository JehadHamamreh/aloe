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
%               of N elements (N=72, 180, ...)
%
% points        - Number of (fft/ifft) points
%
% direction     - 0: fft, 1: ifft
%
% Outputs:     
% out           - 1D vector of 'points' samples, complex-valued
%               

% Spec:         3GPP TS 36.211 section 6.12 v10.5.0
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:1.1.2013
% 

% OFDM modulator

function out = ofdm_modem(in, ofdm_symbols_per_subframe, points, sc, direction)

    %% Parameters   
    M = sc;             % TX: input data lenght per OFDM Symbol, RX: output lenght per OFDM Symbol
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
    
    for k=0:(ofdm_symbols_per_subframe-1)
        
        if (direction)
            
            input_symbols = in(k*M+1:(k+1)*M);
            
            %% set fft/ifft input points to data samples and zeros according to standard
            % first fft/ifft input (DC) is zero
            input(1) = 0;                       
    
            % upper half of the input data symbols assigned to M_half consecutive fft/ifft points
            for i=1:M_half % C: 0..M_half-1
                input(i+1) = input_symbols(M_half+i);
            end
    
            % zero-padding, 1st part: (nof_zeros_half-1) zeros
            for i=(M_half+1):(N_half-1)
                input(i+1) = 0;
            end
    
            % zero-padding, 2nd part: nof_zeros_half zeros
            for i=N_half:(upper_index-1)
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
            output = ifft(input); % ifft (TX) 
            out(k*N+1:(k+1)*N) = output;
        else
            output = fft(input);  % fft (RX)
            out(k*M+1:(k+1)*M) = [output(upper_index+1:N), output(2:M_half+1)]; % Unpadding
        end
    end
   
%     plot(abs(output));
%     figure;
%     plot(abs(fft(output)));

end

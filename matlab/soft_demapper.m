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

%%%%%% Modulation Demapping, hard and soft %%%%%%
% Inputs:
% in            - vector of complex symbols
%
% modulation    - modulation type
%
% soft          - 0: hard decision; 1: LLR; 2: approximate LLR
%
% sigma2        - Noise vairance
%
% Outputs:
% out           - vector of soft or hard decision bits
%

function out = soft_demapper(in, modulation, soft, sigma2)

% Configuration
inf = 100;          % -inf indicates a strong or hard '0' , inf a strong/hard '1' (ard demapping)
change_sign = -1;   % factor changing the sign. The LLR is positive when you are "closer" to a '0' then a '1'. The decoder assumes the opposite.

matlab_constellation = 0;  % set to 1 when testing against Matlab demodulators
% This parameter can be omitted for C implementation

% modulation parameters
BPSK = 0;
QPSK = 1;
QAM16 = 2;
QAM64 = 3;


switch modulation
    case BPSK,  M = 2; % M: number of constellation points
    case QPSK,  M = 4;
    case QAM16, M = 16;
    case QAM64, M = 64;
end;

% parameter 'matlab_constellation' may be omitted or set to 0 for C implementation
[bpsk_table, S0_bpsk, S1_bpsk, qpsk_table, S0_qpsk, S1_qpsk, qam16_table, S0_qam16, S1_qam16, qam64_table, S0_qam64, S1_qam64] = soft_constellation_tables(matlab_constellation);

% demodulation parameters
sigma2 = 1;  % noise variance

%%% DEMODULATION %%%
if (soft == 0)  % HARD DEMODULATION: demodulation is specific for the given symbol constellation
    switch modulation
        case BPSK
            out = hard_bpsk(in, inf);
        case QPSK
            out = hard_qpsk(in, inf);
        case QAM16
            out = hard_qam16(in, inf, matlab_constellation);
        case QAM64
            out = hard_qam64(in, inf, matlab_constellation);
    end
    
else % SOFT DEMODULATION - LLR: (initial) algorithm (implementation -- non-optimized) is regular, independent of symbol constellation
    switch modulation
        case BPSK
            constellation = bpsk_table; S0 = S0_bpsk; S1 = S1_bpsk;
        case QPSK
            constellation = qpsk_table; S0 = S0_qpsk; S1 = S1_qpsk;
        case QAM16
            constellation = qam16_table; S0 = S0_qam16; S1 = S1_qam16;
        case QAM64
            constellation = qam64_table; S0 = S0_qam64; S1 = S1_qam64;
    end
    
    if (soft == 1)  % exact LLR
        out = change_sign*llrexact(in, constellation, S0, S1, sigma2);
    else            % approximate LLR
        out = change_sign*soft_llrapprox(in, constellation, S0, S1, sigma2);
    end
end

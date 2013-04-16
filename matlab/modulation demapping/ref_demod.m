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

% Author: 	Vuk Marojevic (vuk.marojevic@gmail.com)
% Last revision:16.1.2013 

% Reference Demodulator using available Matlab functions (toolboxes)
% ==================================================================
% Parameters:
% M: Number of constellation symbols
% 

function out = ref_demod(in, M, soft, sigma2)

    % Assumtion: from left-upper symbol columnwise (first down, than right)
    % BPSK:
	%    Q
	%    |  0
	%---------> I
	% 1  |
    bpsk_mapping = [1, 0]; % column-wise from top-left to bottom-right

    % QPSK:
    % LTE-QPSK constellation:
	%     Q
	% 10  |  00
	%-----------> I
	% 11  |  01
    qpsk_mapping = [2, 3, 0, 1];  % column-wise from top-left to bottom-right

    % 16QAM:
    % LTE-16QAM constellation:
	%                Q
	%  1011	   1001  |	 0001	0011
	%  1010	   1000  |	 0000	0010
	%---------------------------------> I
	%  1110    1100  |   0100	0110
	%  1111    1101  |   0101	0111
    % mapping column-wise from top-left to bottom-right:
    qam16_mapping = [11, 10, 14, 15, 9, 8, 12, 13, 1, 0, 4, 5, 3, 2, 6, 7];

    % 64QAM:
    %  47-101111    45-101101   37-100101     39-100111   |     7-000111    5-000101    13-001101   15-001111
    %  46-101110    44-101100   36-100100     38-100110   |     6-000110    4-000100    12-001100   14-001110
	%  42-101010    40-101000   32-100000     34-100010   |     2-000010	0-000000    8-001000    10-001010
	%  43-101011    41-101001   33-100001     35-100011   |     3-000011	1-000001    9-001001    11-001011
	%-------------------------------------------------------------------------------------------------------------> I
    %  59-111011    57-111001   49-110001     51-110011   |     19-010011   17-010001   25-011001   27-011011
    %  58-111010    56-111000   48-110000     50-110010   |     18-010010   16-010000   24-011000   26-011010
	%  62-111110    60-111100   52-110100     54-110110   |     22-010110	20-010100   28-011100   30-011110
	%  63-111111    61-111101   53-110101     55-110111   |     23-010111   21-010101   29-011101   31-011111
    % mapping column-wise from top-left to bottom-right:
    qam64_mapping = [47, 46, 42, 43, 59, 58, 62, 63, 45, 44, 40, 41, 57, 56, 60, 61, 37, 36, 32, 33, 49, 48, 52, 53, 39, 38, 34, 35, 51, 50, 54, 55, ...
    7, 6, 2, 3, 19, 18, 22, 23, 5, 4, 0, 1, 17, 16, 20, 21, 13, 12, 8, 9, 25, 24, 28, 29, 15, 14, 10, 11, 27, 26, 30, 31];

    switch (M) 
        case 2,     S = bpsk_mapping;   P = pi/8;   % 0.707+0.707j, -0.707-0.707j (OK, OK)
        case 4,     S = qpsk_mapping;   P = 0;      % 1+/-j             -> LTE: 1/sqrt(2)*(1+/-j)
        case 16,    S = qam16_mapping;  P = 0;      % 1|3 * (1+/-j)     -> LTE: 1|3 * 1/sqrt(10)*(1+/-j)
        case 64,    S = qam64_mapping;  P = 0;      % 1|3|5|7 * (1+/-j) -> LTE: 1|3|5|7 * 1/sqrt(42)*(1+/-j)
    end;

    % Create demodulater object 'h' to be used with demodulation function
    % - 'DecisionType', 'hard decision', 'OutputType', 'integer'
    % - 'DecisionType', 'llr', 'OutputType', 'bit'
    % - 'DecisionType', 'approximate llr', 'OutputType', 'bit'
    if (soft == 0) % hard decision
        h = modem.qamdemod('M', M, 'PhaseOffset', P, 'SymbolOrder',...
        'user-defined', 'SymbolMapping', S, 'OutputType', 'integer', 'DecisionType',...
        'hard decision', 'NoiseVariance', sigma2)
    elseif (soft == 1) % exact LLR
         h = modem.qamdemod('M', M, 'PhaseOffset', P, 'SymbolOrder',...
         'user-defined', 'SymbolMapping', S, 'OutputType', 'bit', 'DecisionType',...
         'llr', 'NoiseVariance', sigma2)
    else % approximate LLR
        h = modem.qamdemod('M', M, 'PhaseOffset', P, 'SymbolOrder',...
        'user-defined', 'SymbolMapping', S, 'OutputType', 'bit', 'DecisionType',...
        'approximate llr', 'NoiseVariance', sigma2)
    end
    
    % use another Matlab function to demodulate symbols with specific (LTE) constellation
    % requires changes in plotting symbol labels below, since h.SymbolMapping is not defined
    if (M == 4) % QPSK
        C = 1/sqrt(2)*[1+1j, 1-1j, -1+1j, -1-1j];
        if (soft == 0)
            h = modem.genqamdemod('Constellation', C, 'OutputType', 'integer',...
             'DecisionType', 'hard decision', 'NoiseVariance', sigma2)
        elseif (soft == 1) % exact LLR
            h = modem.genqamdemod('Constellation', C, 'OutputType', 'bit',...
             'DecisionType', 'llr', 'NoiseVariance', sigma2)
        elseif (soft == 2) % approx. LLR
            h = modem.genqamdemod('Constellation', C, 'OutputType', 'bit',...
             'DecisionType', 'approximate llr', 'NoiseVariance', sigma2)
        end
    end
    
    scatterPlot = commscope.ScatterPlot('SamplesPerSymbol', 1, 'Constellation', h.Constellation);
    % Show constellation
    scatterPlot.PlotSettings.Constellation = 'on';
    scatterPlot.PlotSettings.ConstellationStyle = 'rd';
    
    % Add symbol labels
    hold on;
    k=log2(h.M);
    for jj=1:h.M
            if (M==4)
                text(real(h.Constellation(jj))-0.15,imag(h.Constellation(jj))+0.15, dec2base(jj-1,2,k));
            else
                text(real(h.Constellation(jj))-0.15,imag(h.Constellation(jj))+0.15, dec2base(h.SymbolMapping(jj),2,k));
            end
    end
    hold off;

    if (soft > 0)
        change_sign = -1;
    else
        change_sign = 1;
    end
    out = change_sign*demodulate(h, in);
    % if QPSK hard, e.g.: 0 = "00", 1 = "01", 2 = "10", 3 = "11"
    
    %out_bin = zeros(length(in),k);
    if (soft == 0)
        i=1:length(in);
        out_bin(i,:) = dec2base(out(i),2,k)
    end
end

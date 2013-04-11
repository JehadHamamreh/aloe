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

% Soft demapping based on exact log-likelihood ratio (LLR) calculation
% ====================================================================

function out = llrexact(in, symbols, S0, S1, sigma2)

N = length(symbols);    % # constellation points
B = log2(N);            % # bits per symbol
M = length(in);         % # received symbols for demodulation

%out = zeros(M,B);      % write out in blocks of B bits
out = zeros(M*B,1);     % write out as continuous bitstream

for s=1:M   % symbol
    for b=1:B  % bit
        num = 0;    % numerator of LLR
        den = 0;    % denominator of LLR
        for i=1:N/2 % half the symbols have a '1' and half a '0' at any position
            num = num + exp(-((real(in(s)) - real(symbols(S0(b,i))))^2 + (imag(in(s)) - imag(symbols(S0(b,i))))^2) / sigma2);
            den = den + exp(-((real(in(s)) - real(symbols(S1(b,i))))^2 + (imag(in(s)) - imag(symbols(S1(b,i))))^2) / sigma2);
        end
        % out(s,b) = log(num/den);      % write out in blocks of B bits
        out((s-1)*B+b) = log(num/den);  % write out as continuous bitstream
    end
end

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

function in = generate_symbols(modulation, N, matlab_constellation)

% modulation parameters
BPSK = 0;
QPSK = 1;
QAM16 = 2;
QAM64 = 3;

q = 0.5;    % extends the ideal constellation diagram by q in the x and y direction
bpsk_max = 1/sqrt(2)+q;
qpsk_max = 1/sqrt(2)+q;
if matlab_constellation
    qam16_max = 3+q;
    qam64_max = 7+q;
else
    qam16_max = 3/sqrt(10)+q;
    qam64_max = 7/sqrt(42)+q;
end

in = zeros(N,1);

for i=1:N
    if (modulation == BPSK)
        x = rand(N,1) - 0.5;    % generate N real values, each between -0.5 and +0.5
        y = rand(N,1) - 0.5;    % generate N real values, each between -0.5 and +0.5
        in = 2*bpsk_max*(x + j*y);
    elseif (modulation == QPSK)
        x = rand(N,1) - 0.5;
        y = rand(N,1) - 0.5;
        in = 2*qpsk_max*(x + j*y);
    elseif (modulation == QAM16)
        x = rand(N,1) - 0.5;
        y = rand(N,1) - 0.5;
        in = 2*qam16_max*(x + j*y);
    elseif (modulation == QAM64)
        x = rand(N,1) - 0.5;
        y = rand(N,1) - 0.5;
        in = 2*qam64_max*(x + j*y);
    end
end
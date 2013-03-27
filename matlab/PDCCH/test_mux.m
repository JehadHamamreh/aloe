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


% Test PDCCH multiplexing

Nreg = 63;  % Number of available resource-element groups for PDCCH, should at be least larger than sum(M)/8 (8 bits per resource element group)
n = 3;      % number of PDCCH to be multiplexed
input = cell(n,1);

M(1) = 72;
M(2) = 72;
M(3) = 144;
for i=1:n
%    M(i) = (2^mod((i-1),3))*72;    % set number of bits on each PDCCH to (2^x)*72, x = 0, 1, 2, 3.
    input{i,1} = randi(2,1,M(i))==1;
end

fprintf('\nMinimum necessary Nreg = %f.\n',sum(M)/8);
out = pdcch_mux(input, n, Nreg)
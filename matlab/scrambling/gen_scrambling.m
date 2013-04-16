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


% Test gen_scrambling
addpath('/home/vuk/DATOS/workspace/gen_scrambling');

M = 160;

data = randi(2,1,M)==1;
c = randi(2,1,M)==1;

in = cell(2,1); % create a cell array of two fields
in{1,1} = data; % generate a random logical stream, M data samples (bits)
in{2,1} = c;    % generate a random logical stream, scrambling sequence of M samples (bits)

direct = 1;

out = mod((data+c),2);   % scramble input sequence with scrambling sequence
out_mex = am_gen_scrambling(in,{{'direct',int32(direct)}});



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
% Last revision:1.1.2013 


% Defines test vectors for testing Layer Mapping, Precoding, Un-Precoding, 
% and Layer Demapping

in=rand(1,24)+i*rand(1,24);
% Single Antenna Port
[out0_0, out1_0] = test_layer(in,0);


% Transmit Diversity
% q=1 codeword
% v=2 layers
% ap=2 antenna ports
[out0_1, out1_1] = test_layer(in,1);


% Transmit Diversity
% q=1 codeword
% v=4 layers
% ap=4 antenna ports
[out0_2, out1_2] = test_layer(in,2);


in=rand(1,30)+i*rand(1,30);
% Transmit Diversity
% q=1 codeword
% v=4 layers
% ap=4 antenna ports
[out0_3, out1_3] = test_layer(in,2);


% Spatial Multiplexing, Precoding using antenna ports with UE-specific
% reference signals
% q=1 codeword
% v=2 layers
% ap=2 antenna ports
[out0_4, out1_4] = test_layer(in,4);


% Spatial Multiplexing, Precoding using antenna ports with UE-specific
% reference signals
% q=1 codeword
% v=6 layers
% ap=6 antenna ports
[out0_5, out1_5] = test_layer(in,11);


% Spatial Multiplexing, Precoding using antenna ports with UE-specific
% reference signals
% q=2 codeword
% v=7 layers
% ap=7 antenna ports
[out0_6, out1_6] = test_layer(in,12);

in=rand(1,49)+i*rand(1,49);
[out0_7, out1_7] = test_layer(in,12);


% Spatial Multiplexing, Precoding using antenna ports with UE-specific
% reference signals
% q=2 codeword
% v=8 layers
% ap=8 antenna ports
in=rand(1,64)+i*rand(1,64);
[out0_8, out1_8] = test_layer(in,13);

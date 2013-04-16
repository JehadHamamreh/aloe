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


% Generates test signal for testing Layer Mapping and Demapping

function [out0, out1] = test_layer(in, mode)

    addpath('../common functions');
    
    fprintf('\nTest Layer Mapper, Precoder, Un-Precoder, and Layer Demapper for LTE DL\n');
    out0 = 0;
    out1 = 0;
    Ms = length(in);
    
    % styles:
    single_ap = 0;
    tx_div = 1;
    sp_mux = 2;
    
    % Precoding for spatial multiplexing using antenna ports with UE-specific
    % reference signals: 
    sm_UE = 4;          % (Section 6.3.4.4)

    % Mapping of complex valued symbols on q = 1 or 2 codewords to up to
    % v = 8 layers
    % Valid combinations
    ...
    % ...single antenna port:
    if (mode==0)
        fprintf('Single antenna port.\n');
        style = single_ap;
        precoding_style = single_ap;
        v=1         % # layers
        q=1         % # codewords
        p=0;        % antenna port vector
        in0 = in;   % input data on codeword 0 (1st codeword)
        in1 = 0;    % input data on codeword 1 (2nd codeword), if applicable

    % ...transmit diversity:
    elseif (mode==1)
        fprintf('Transmit diversity with q codewords and v layers.\n');
        style = tx_div;
        precoding_style = tx_div;
        v=2
        q=1
        p=[0, 1];
        in0 = in;
        in1 = 0;
    
    elseif (mode==2)
        fprintf('Transmit diversity q codewords and v layers.\n');
        style = tx_div;
        precoding_style = tx_div;
        v=4
        q=1
        p=[0, 1, 2, 3];
        in0 = in;
        in1 = 0;   

    % ...spatial multiplexing:
    elseif (mode==3)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux; % this is actually 'single_ap'
        precoding_style = single_ap;
        v=1
        q=1
        p=0;
        in0 = in;
        in1 = 0;

    elseif (mode==4)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=2
        q=1
        i=7:(v+6); p=[i]
        in0 = in;
        in1 = 0;
        
    elseif (mode==5)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=2
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/2) ~= Ms/2)
            fprintf('ERROR: Number of test samples not integer divisible by 2.\n');
            return;
        end
        in0 = in(1:(Ms/2));
        in1 = in((Ms/2+1):Ms);
    
    elseif (mode==6)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=3
        q=1
        i=7:(v+6); p=[i]
        in0 = in;
        in1 = 0;
    
    elseif (mode==7)
        fprintf('Spatial multiplexing.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=3
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/3) ~= Ms/3)
            fprintf('ERROR: Number of test samples not integer divisible by 3.\n');
            return;
        end
        Ms0 = Ms/3;
        Ms1 = 2*(Ms/3);
        in0 = in(1:Ms0);
        in1 = in(Ms0+1:Ms);
    
    elseif (mode==8)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=4
        q=1
        i=7:(v+6); p=[i]
        in0 = in;
        in1 = 0;

    elseif (mode==9)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=4
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/2) ~= Ms/2)
            fprintf('ERROR: Number of test samples not integer divisible by 2.\n');
            return;
        end
        Ms0 = Ms/2;
        Ms1 = Ms/2;
        in0 = in(1:Ms0);
        in1 = in(Ms0+1:Ms);

    elseif (mode==10)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=5
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/5) ~= Ms/5)
            fprintf('ERROR: Number of test samples not integer divisible by 5.\n');
            return;
        end
        Ms0 = 2*(Ms/5);
        Ms1 = 3*(Ms/5);
        in0 = in(1:Ms0);
        in1 = in(Ms0+1:Ms);
        
    elseif (mode==11)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=6
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/2) ~= Ms/2)
            fprintf('ERROR: Number of test samples not integer divisible by 2.\n');
            return;
        end
        Ms0 = Ms/2;
        Ms1 = Ms/2;
        in0 = in(1:Ms0);
        in1 = in(Ms0+1:Ms);
        
    elseif (mode==12)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=7
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/7) ~= Ms/7)
            fprintf('ERROR: Number of test samples not integer divisible by 7.\n');
            return;
        end
        Ms0 = 3*(Ms/7);
        Ms1 = 4*(Ms/7);
        in0 = in(1:Ms0);
        in1 = in(Ms0+1:Ms);

    elseif (mode==13)
        fprintf('Spatial multiplexing q codewords and v layers.\n');
        style = sp_mux;
        precoding_style = sm_UE;
        v=8
        q=2
        i=7:(v+6); p=[i]
        if (floor(Ms/2) ~= Ms/2)
            fprintf('ERROR: Number of test samples not integer divisible by 2.\n');
            return;
        end
        Ms0 = Ms/2;
        Ms1 = Ms/2;
        in0 = in(1:Ms0);
        in1 = in(Ms0+1:Ms);
       
    else
        fprintf('ERROR: Wrong mode chosen. Please choose a valid mode: 0 (single antenna port), 1 or 2 (transmit diversity), or 3-13 (spatial multiplexing).\n');
        return;
    end
        
    out_layer_mapper = lte_PDSCH_layer_mapper(in0, in1, v, q, style)
    out_precoding = lte_PDSCH_precoding(out_layer_mapper, p, precoding_style)
    out_unprecoding = lte_PDSCH_unprecoding(out_precoding, v, precoding_style)
    [out0, out1] = lte_PDSCH_layer_demapper(out_unprecoding, v, q, style)
    
end

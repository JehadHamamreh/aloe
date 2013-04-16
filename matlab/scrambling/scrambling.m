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

%%% LTE Scrambling %%%
% For each codeword q, the block of bits transmitted on the physical 
% channel in one subframe shall be scrambled prior to modulation

% The scrambling sequence generator shall be initialised at the start of 
% each subframe

% DL-SCH: Transport-channel-specific scrambling using length-31 
% Gold sequences
% UL-SCH: UE-specific scrambling for interference randomization
% 

% Inputs:
% in        - input bit sequence (stream of '0s' and '1s', chars in C?)
%
% desc      - Scrambling/descrambling indicator (0: scrambling, 1:
%           descrambling)
%
% UL        - if set (UL>0): scrambling/descrambling of UL Tx/Rx
% 
% x, y      - Tags (1D vectors indicating bit positions of input bit sequence)
%           only for UL
%
%
% The following parameters are set here internally:
% cell_gr   - Cell ID group index (integer [0,2])
%
% cell_sec  - cell ID sector index within the physical-layer
%           cell-identity group (integer [0, 167])
%
% nRNTI     - Radio network temporary identifier (integer [0, 2e16-1=65535])
%
% ns        - Subframe (integer [0, 1, ..., 9])
%
% Outputs:
% out       - scrambled bit sequence
%


% Spec:         3GPP TS 36.211 section 6.3.1 v10.5.0
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:30.1.2013
% 


function out = scrambling(samples, desc, UL, x, y)

    %addpath('/home/vuk/DATOS/workspace/lte_scrambling');
    addpath('/usr/local/mex');
    addpath('../common functions');    

    in = set_input(samples, desc);
    M = length(in);
    out = zeros(1,M);
    %out_mex = zeros(1,M);
   
    % UL: 'x' and 'y' are placeholders to scramble the HARQ-ACK bits in a 
    % way that maximizes the Euclidean distance of the modulation symbols 
    % carrying HARQ-ACK information. */
    % HARQ ACK Indication is coded, interleaved with data before scrambling.
    
    % Pseudorandom scrambling sequence generation parameters
    q =         0;
    ns =        5;      % slot. range: integer in [0, 19], default: 0
    cell_gr =   2;      % Physical-layer cell-identity group, range: integer in [0, 167], default: 0
    cell_sec =  0;      % Physical-layer identity within the physical-layer identity group, range: integer in [0, 2], default: 0
    nRNTI =     65530;  % range: integer in [0, 65535], default: 0
    N_MBSFN =   0;      
    channel =   3
    direct =    0;
    sample = 0;         % if channel==3 
%     channel = 0; % PDSCH/PUSCH
%     channel = 1; % PCFICH
%     channel = 2; % PDCCH
%     channel = 3; % PBCH
%     channel = 4; % PMCH
%     channel = 5; % PUCCH
    
    c = scrambling_sequence_gen(q, M, cell_gr, cell_sec, nRNTI, N_MBSFN, ns, channel);
    x_size = length(x) 
    y_size = length(y)

    if (desc == 0) % scrambling
        
        out = mod((in+c'),2);   % scramble input sequence with scrambling sequence
        
        if (UL == 1)
            for i=1:x_size
                if (x(i)>0)
                    out(x(i)) = 1;  % ACK/NACK or Rank Indication placeholder bits
                end
            end
            for i=1:y_size
                if (y(i)>1)
                    out(y(i)) = out(y(i)-1);% ACK/NACK or Rank Indication repetition placeholder bits
                end
            end             
        end
        out_mex = am_lte_scrambling(in, {{'direct',int32(direct)},{'q',int32(q)},{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'nrnti',int32(nRNTI)},{'channel',int32(channel)}});
        %out_mex = out_mex

    elseif (desc == 1) % soft scrambling % descrambling (soft inputs, i.e. floats rather than bits)
 
        x_count = 1;
        y_count = 1;       
        for i=1:M
            if ((UL == 1) && ((i == x(x_count)) || (i == y(y_count))))
                out(i) = in(i); % do not modify placeholder bits 'x' and 'y' (correct?)
                if (i == x(x_count))
                    x_count = x_count+1;
                else
                    y_count = y_count+1;
                end
            elseif (((in(i) > 0) && (c(i) == 1)) || ((in(i) < 0) && (c(i) == 1)))
                % (1) would return a 0 in case of hard demod
                % (2) would return a 1 in case of hard demod
                out(i) = -in(i);
            else
                out(i) = in(i);
            end
        end
        %out_mex = am_lte_descrambling(in, {{'q',int32(q)},{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'nrnti',int32(nRNTI)},{'channel',int32(channel)}});
        out_mex = am_lte_scrambling(in, {{'direct',int32(direct)},{'hard',int32(0)},{'q',int32(q)},{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'nrnti',int32(nRNTI)},{'channel',int32(channel)}});
   
    else    % hard descramabling
        
        x_count = 1;
        y_count = 1;       
        for i=1:M
            if ((UL == 1) && ((i == x(x_count)) || (i == y(y_count))))
                out(i) = in(i); % do not modify placeholder bits 'x' and 'y' (correct?)
                if (i == x(x_count))
                    x_count = x_count+1;
                else
                    y_count = y_count+1;
                end
            else
                out = mod((in+c'),2);   % scramble input sequence with scrambling sequence
            end
        end
        %out_mex = am_lte_hard_descrambling(in, {{'q',int32(q)},{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'nrnti',int32(nRNTI)},{'channel',int32(channel)}});
        out_mex = am_lte_scrambling(in, {{'direct',int32(direct)},{'q',int32(q)},{'subframe',int32(floor(ns/2))},{'cell_gr',int32(cell_gr)},{'cell_sec',int32(cell_sec)},{'nrnti',int32(nRNTI)},{'channel',int32(channel)}});
         
    end

    P=M;
    plot(1:P, out(1:P), 'r:o', 1:P, out_mex(1:P), 'b--+');
end

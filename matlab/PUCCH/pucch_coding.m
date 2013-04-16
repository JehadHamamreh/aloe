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
%  * but WITHOUT ANY WARRANTY; without even the implied warranty o
%  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  * GNU Lesser General Public License for more details.
%  * 
%  * You should have received a copy of the GNU Lesser General Public License
%  * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
%  */

% Uplink Control Information (UCI) on the LTE Physical Uplink Control Channel (PUCCH)

% Data arrives to the coding unit in the form of indicators for measurement 
% indication, scheduling request and HARQ acknowledgement.

% Three forms of channel coding are used, one for the channel quality 
% information CQI/PMI, another for HARQ-ACK (acknowledgement) and scheduling 
% request and another for combination of CQI/PMI and HARQ-ACK.

% Reference: 3GPP TS 136 212, V 10.6.0
% mode 0: Channel coding for UCI HARQ-ACK, Section 5.2.3.1 -- not implemented
% mode 1: Channel coding for UCI, Section 5.2.3.2 -- not yet implemented
% mode 2: Channel coding for UCI channel quality information -- see below
% mode 3: Channel coding for UCI channel quality information and HARQ-ACK
% -- not implemented


function output = pucch_coding(in, mode, M, iM, direction)
    
    TX = 1;
    RX = 0;
    A = length(in);
    [x,y] = size(iM);
    
    if (mode == 2)
        if (direction == TX)
            output = mod(in*M(:,1:A).',2);
        else
            [value, output] = max((in*iM.')+((ones(1,A)-in)*(ones(x,y)-iM).'))
        end
    else
        output = {};    % other modes not yet implemented
    end
    
end

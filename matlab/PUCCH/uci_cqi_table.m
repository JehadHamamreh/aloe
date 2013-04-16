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

% Uplink Control Information (UCI) on the LTE Physical Uplink Control 
% Channel (PUCCH)

% Coding table for for the channel quality information CQI/PMI.

% Reference: 3GPP TS 136 212, V 10.6.0, Section 5.2.3.3


function M = uci_cqi_table
    
    M = [1 1 0 0 0 0 0 0 0 0 1 1 0;
         1 1 1 0 0 0 0 0 0 1 1 1 0;
         1 0 0 1 0 0 1 0 1 1 1 1 1;
         1 0 1 1 0 0 0 0 1 0 1 1 1;
         1 1 1 1 0 0 0 1 0 0 1 1 1;
         1 1 0 0 1 0 1 1 1 0 1 1 1;
         1 0 1 0 1 0 1 0 1 1 1 1 1;
         1 0 0 1 1 0 0 1 1 0 1 1 1;
         1 1 0 1 1 0 0 1 0 1 1 1 1;
         1 0 1 1 1 0 1 0 0 1 1 1 1;
         1 0 1 0 0 1 1 1 0 1 1 1 1;
         1 1 1 0 0 1 1 0 1 0 1 1 1;
         1 0 0 1 0 1 0 1 1 1 1 1 1;
         1 1 0 1 0 1 0 1 0 1 1 1 1;
         1 0 0 0 1 1 0 1 0 0 1 0 1;
         1 1 0 0 1 1 1 1 0 1 1 0 1;
         1 1 1 0 1 1 1 0 0 1 0 1 1;
         1 0 0 1 1 1 0 0 1 0 0 1 1;
         1 1 0 1 1 1 1 1 0 0 0 0 0;
         1 0 0 0 0 1 1 0 0 0 0 0 0];
    
end

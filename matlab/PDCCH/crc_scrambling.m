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


% scrambles the crc parity bits with the rnti and antenna selection mask if
% the case. The case is:
% - UE transmit antenna selection not configured or applicable
%   - DC format 0: scrambling with rnti & antenna selection mask
%   - other DC format: scrambling with rnti only
% - UE transmit antenna selection configured and applicable & DCI format 0:
%   scrambling with rnti & antenna selection mask
function out = crc_scrambling(in, antenna_selection, dci_format, rnti, ue_port)
    
    L = 16;
    B = length(in);
    
    if ((antenna_selection == 0) || (dci_format == 0)) 
        % antenna selection not configured or applicable || dci_format 0
        crc_scrambling = 1;
        c = zeros(1,L);
        for i=0:L-1
            c(L-i) = bitand(rnti,2^i)>0;
        end
    end
    if ((antenna_selection == 1) &&  (dci_format == 0)) 
        % antenna selection configured and applicable && dci_format 0
        crc_scrambling = 1;
        c(L) = c(L)+ue_port;
    end
    
    if (crc_scrambling) % crc scrambling applies
        out(1:B-L) = in(1:B-L);
        l=B-L+1:B;
        l0=1:L;
        out(l) = mod((in(l)+c(l0)),2);   % scrambling of parity bits
    else
       out = in; 
    end
    
end

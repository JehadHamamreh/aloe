% /* 
%  * Copyright (c) 2012-2013, Vuk Marojevic <vuk.marojevic@gmail.com>.
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

% Control Format Indicator (CFI) coding. Determines the 32-bit sequence of
% as a function of the CFI.

%% Parameters
% Inputs:
% cfi           - Control Format Indicator (CFI): 1, 2, or 3
%
%
% Outputs:
% out           - Coded CFI (32-bit sequence)
%

% Spec:         3GPP TS 36.212, v10.6.0 section 5.3.4.1
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:18.2.2013
% 

function out = cfi_coding(cfi)

    switch (cfi)
        case 1
            aux = [0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1];
        case 2 
            aux = [1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0];
        case 3
            aux = [1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1];
        case 4 % reserved
            out = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
        otherwise
            fprintf('\nError: Wrong CFI. Please specify a valid value (1, 2 or 3).\n');
            return;
    end
    
    % make it logical
    for i=1:32
        out(i) = aux(i)==1;
    end
    
end


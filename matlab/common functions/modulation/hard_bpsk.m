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

% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last revision:16.1.2013 

% Hard modulation demapping BPSK
% ==============================
% zero indicates the value assigned to '0', and one the value assigned to '1'

% INPUTS
% in:       input vector of complex symbols
% zero, one:      real value, e.g. -100, 100, onto which a '0' and '1' is mapped
% 
% OUTPUTS
% out:      output matrix of length(in) rows and 1 colum, where each
%           element represents a (soft or hard decision) bit
% 
function out = hard_bpsk(in, zero, one)

M = length(in);
out = zeros(M,1);

for i=1:M
    if (real(in(i)) > 0 && imag(in(i)) > 0)    % 1st Quadrant
        out(i) = -inf;
    elseif (real(in(i)) > 0 && imag(in(i)) < 0)    % 4th Quadrant
        if (real(in(i)) > -imag(in(i)))
            out(i) = zero;
        else
            out(i) = one;
        end
    elseif (real(in(i)) < 0 && imag(in(i)) > 0)    % 2nd Quadrant
        if (imag(in(i)) > -real(in(i)))
            out(i) = zero;
        else
            out(i) = one;
        end
    else    % 3rd Quadrant
        out(i) = inf;
    end
end

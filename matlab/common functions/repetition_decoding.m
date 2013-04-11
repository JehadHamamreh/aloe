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


% Decoding of coded information based on bit repetition

function out = repetition_decoding(input, rep, soft)

    S = length(input);
    D = floor(S/3);
 
    if (soft)
        threshold = 0;
    else
        threshold = floor(rep/2);
    end
    
    out = zeros(1,D);

    i=1;
    for j=1:rep:S
        if (sum(input(j:j+rep-1)) > threshold)
            out(i) = 1;
        else
            out(i) = 0;
        end
        i = i+1;
    end

end
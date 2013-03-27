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


% Sub-block Deinterleaver for Rate Unmatching module
% Only works for PDCCH, not PDSCH

% Inputs:
% in        - Input bit streams (3 streams of '0s' and '1s'), which are the
%           3 output streams of the encoder, 3xD-array of bits
%
% Outputs:
% out       - output stream


function dx_rx = subblock_deinterleaver(vx, turbo)

    V = length(vx);     % # input bits
    cols = 32;          % # columns
    rows = ceil(V/cols);% # rows 
    Kp = rows*cols;
    vxx = zeros(1,Kp);  % inteleaver intput
    dxx = zeros(1,Kp);  % inteleaver output
    dx_rx = zeros(1,V); % output
    
    
	% 1. Define inter-column permutation matrix and dummy-bit insertion pattern
    if (turbo)
        % GPP for turbo-encoded channels:
        %P = [0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31];
        % Alternative (optimized) implementation employs P(i) = mod(P_GPP(i)+1,cols):
        P = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 2, 18, 10, 26, 6, 22, 14, 30, 4, 20, 12, 28, 8, 24, 16, 0];        
        % Pattern for inserting dummy bits:
        Pd = zeros(1,30);
        for i=0:31
            Pd(P(i+1)+1) = i;
        end
    else
        P = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30];
        % Pattern for inserting dummy bits:
        Pd = [16, 0, 24, 8, 20, 4, 28, 12, 18, 2, 26, 10, 22, 6, 30, 14, 17, 1, 25, 9, 21, 5, 29, 13, 19, 3, 27, 11, 23, 7, 31, 15];
    end
    %Pd
    nof_dummybits = Kp-V;
    if (nof_dummybits > 0)
        for i=0:nof_dummybits-1
            vxx(Pd(i+1)*rows+1) = -1;
        end
        idx = 0;
        for j=0:Kp-1
            if (vxx(j+1)==-1)
                idx = idx+1;
            else
                vxx(j+1) = vx(j-idx+1);
            end
        end
    else
        vxx = vx;
    end
    
    % 2. Construct interleaver matrix m
    m = zeros(rows,cols);
    % write column-wise (column by column)
    for i=0:cols-1
        m(:,i+1) = vxx(i*rows+1:(i+1)*rows);  % C: m(:,i) = dxx(i*rows:(i+1)*rows-1);  
    end
    
	% 3. Permute columns of m
    m_perm = zeros(rows,cols);
    % reverse permutation, column-wise
    for i=0:cols-1
        m_perm(:,P(i+1)+1) = m(:,i+1);  % +1 for Matlab (all 3 occurences)
    end
    
    % 4. Read out row-wise (row by row)
    for i=0:rows-1
        dxx(i*cols+1:(i+1)*cols) = m_perm(i+1,:);
    end
    
    % 5. Remove <NULL>-bits if any
    dx_rx = dxx(nof_dummybits+1:Kp);
    
end

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


% Sub-block Interleaver for PDCCH Rate Matching


% Inputs:
% in        - Input bit streams (3 streams of '0s' and '1s'), which are the
%           3 output streams of the encoder, 3xD-array of bits
%
% Outputs:
% out       - output stream


%function vx = subblock_interleaver(dx, stream, turbo)
M = 525;
vx = [(randi(2,1,M)==1);(randi(2,1,M)==1);(randi(2,1,M)==1)];
dx = vx(3,:);
stream = 2;
turbo = 1;

    D = length(dx);
    cols = 32;           % # columns
    rows = ceil(D/cols);  % # rows
    Kp = rows*cols;
    vx = zeros(1,Kp);
    vx1 = zeros(1,Kp);
    vx2 = zeros(1,Kp);
    
    if (Kp > D)
        dxx = [-1*ones(1,Kp-D), dx];   % add dummy or <NULL>-bits, -1 here
    else
        dxx = dx;
    end
    
    % define inter-column permutation pattern
    if (turbo)
        % GPP for turbo-encoded channels
        P = [0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31];
        % Alternative implementation using P(i) = mod(P_GPP(i)+1,cols):
        Px = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 2, 18, 10, 26, 6, 22, 14, 30, 4, 20, 12, 28, 8, 24, 16, 0];
    else
        P = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30];
    end
    
    % permute and write out
%    if ((turbo) && stream == 2)
        p = zeros(1,Kp);
        p2 = zeros(1,Kp);
        for k=0:Kp-1
            % 3GPP:
            p(k+1) = mod(P(floor(k/rows)+1) + cols*(mod(k,rows))+1, Kp);   % '+1'@ p(k+1), P(floor(k/rows)+1) because p and P are indexed from 1 in Matlab.
            
            % same as above, but optimized implementation:
            p2(k+1) = P(floor(k/rows)+1) + cols*(mod(k,rows))+1;   % '+1'@ p(k+1), P(floor(k/rows)+1) because p and P are indexed from 1 in Matlab.
            p2(Kp) = 0;
            
            vx(k+1) = dxx(p(k+1)+1);
            vx1(k+1) = dxx(p2(k+1)+1);
        end
    
%    else
        % Construct interleaving matrix m
        m = zeros(rows,cols);
        % write row-wise (row by row)
        for i=1:rows
            m(i,:) = dxx((i-1)*cols+1:i*cols); % in C: dxx((i-1)*cols:i*cols-1)
        end
        
        % permute columns of m
        m_perm = zeros(rows,cols);
        for i=0:cols-1
            m_perm(:,i+1) = m(:,Px(i+1)+1);  % +1 for Matlab (all 3 occurences)
        end
        
        % read out column-wise (column by column)
        for i=0:cols-1
            vx2(i*rows+1:(i+1)*rows) = m_perm(:,i+1);
        end
        if ((turbo) && (stream == 2))
            % adjust last block of subblock interleaver output for 3rd 
            % stream (stream 2) of turbo-encoded channels
            % this is necessary due to the alternative implementation to
            % match the output of 3GPP: 
            % p(k+1) = mod(P(floor(k/rows)+1)+cols*(mod(k,rows))+1, Kp)
            vx2((cols-1)*rows+1:(i+1)*rows-1) = m_perm(2:rows,cols);
            vx2(Kp) = m_perm(1,cols);
        end
%    end

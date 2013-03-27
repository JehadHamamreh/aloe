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


% PDCCH multiplexing


% The rate matching block creates an output bitstream with a desired code 
% rate. The three bitstreams from the convolutional encoder are interleaved 
% followed by bit collection to create a circular buffer. Bits are selected 
% and pruned from the buffer to create an output bitstream with the desired 
% code rate.


% Inputs:
% input 	- PDCCHs transmitted in one subframe. Cell array of n cells: 
%           input{1,1}, ..., input{n,1}
%
% n         - Number of PDCCH transmitted in one subframe
%
% format    - Vector containing the format of each PDCCH sent in a
%           subframe. Valid PDCCH formats: 0, 1, 2, 3
%
% Nreg      - The number of resource-element groups not
%           assigned to PCFICH or PHICH is N_REG 
%
% Outputs:
% out       - output bit stream: multiplexed input streams


%function out = pdcch_mux(in, n, format, Nreg)
function out = pdcch_mux(input, n, Nreg)

    M = zeros(1,n);     % contains the number of bits on each PDCCH
    format = zeros(1,n);
    max = 576;
    % obtain the number of bits of each PDCCH to be multiplexed
    
    for i=1:n
        %in0 = input{i,1}
        %M(i) = length(input{i,1})
        M(i) = length(input);
        switch M(i)
            case 72,
                format(i) = 0; % break in Matlab exits loops! Not to be used in Matlab to exit a switch case
            case 144,
                format(i) = 1;
            case 288,
                format(i) = 2;
            case 576,
                format(i) = 3;
            otherwise,
                fprintf('\nError: Incorrect number of PDCCH bits of PDCCH channel %d.\n', i);
                return;
        end
%         if (M(i) > max)
%             max = M(i);
%         end
    end
    
    % transform cell arrays to matrix, where each row represents a PDCCH to
    % transmitted in one subframe and to be multiplexed
    in = zeros(n,max);
    for i=1:n
        %in(i,:) = [input{i,1}, zeros(1,max-M(i))];
        in(i,:) = [input(i,:), zeros(1,max-M(i))];
    end
    
%    [streams, M] = size(in);
%    if (streams~=n)
%        fprintf('\nError: Please make sure that the number of PDCCH (n) transmitted per subframe matches the number of input streams.\n');
%        return;
%    end

    % Not sure what values Nreg can take
%	if ((Nreg~=9) || (Nreg~=18) || (Nreg~=36) || (Nreg~=72))
%   	fprintf('\nError: The number of resource element groups should be 9, 18, 36 or 72.\n');
%   	return;
%   end

%     for i=1:n
%         if (length(in(i,:)) < ((format(i)+1)*72))
%             length(in(i,:))
%             fprintf('\nError: Too few input bits on some PDCCH.\n');
%             return;
%         end
%     end
    
    % A PDCCH consisting of n consecutive CCEs may only start on a CCE 
    % fulfilling i mod n = 0 , where i is the CCE number.    
    % Not sure if reordering is necessary.
    % Should be revered at Rx?
%     i=1:n;
%     order = i;
%     if (n>1)
%         for j=1:n
%         for i=1:n-1
%         % order streams in order of increasing formats to make best use of
%         % resources
%             if format(i+1)<format(i);
%                 aux = in(i,:);
%                 f_aux = format(i);
%                 ord_aux = order(i);
%                 in(i,:) = in(i+1,:);
%                 in(i+1,:) = aux;
%                 format(i) = format(i+1);
%                 format(i+1) = f_aux;
%                 order(i) = order(i+1);
%                 order(i+1) = ord_aux;
%             end
%         end
%         end
%     end
%     in
%     format
%     order

    Ncce = floor(Nreg/9);   % Number of available CCEs in the system, numbered from 0 to Ncce-1
    Mtot = 8*Nreg;          % Total number of output bits prior to scrambling
    
    out = zeros(1,Mtot);
    
    fprintf('\nPDDCH format %d.', format);
    i=1:n;
    CCEs = 2.^format(i)     % Number of consecutive CCEs for each of the PDCCHs to be multiplexed: CCEs(i) indicates the necessary number of CCEs for PDCCH i.
    if (sum(CCEs) > Ncce)
        fprintf('\nError: More CCEs necessary a priori than available in the system.\n');
        return;
    end

    % Multiplexing: concatenation of PDCCH
    % A PDCCH consisting of n consecutive CCEs may only start on a CCE 
    % fulfilling i mod n = 0 , where i is the CCE number (not yet occupied 
    % by other PDCCH transmitted in the same subframe, multiplexed here).
    bit=1;      % next free bit | in C: 0
    cce = 0;    % cce is numbered from 0 to Ncce-1
    for i=1:n
        mod_cce = mod(cce,CCEs(i));
        if (mod_cce~=0)
            add_CCEs = CCEs(i)-mod_cce
            out(bit:bit+add_CCEs*72-1) = zeros(1,add_CCEs*72);  % NIL insertion
            bit = bit + add_CCEs*72;
        end
        out(bit:bit+M(i)-1) = in(i,1:M(i));
        cce = cce + CCEs(i);
        bit = bit + M(i);
    end
        
    bit_diff = Mtot - (bit-1); % Difference between desired output bits and actual output bits
    if (bit_diff < 0)
    	fprintf('\nError: Not enough resource-element groups (Nreg) specified for multiplexing the PDCCHs.\n');
        return;
	end
    if (bit_diff > 0)
    	out(bit:Mtot)=zeros(1,bit_diff);  % NIL insertion
    end
    
end

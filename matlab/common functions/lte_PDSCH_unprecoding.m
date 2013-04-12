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

% The precoder takes block of vectors containing complex-valued modulation 
% symbols to generate block of vectors to be mapped to resources on each of
% the antenna ports
% - Previous module: Resource Demapping
% - Next module: Layer Demapping
%
% Downlink (DL) Receiver (PDSCH)
% Different modes: 
% 0: single antenna port,
% 1: Transmit diversity
% 2: spatial multiplexing using antenna ports with cell-specific reference 
%    signals - without cyclic delay diversity (CDD)
% 3: ... - large delay CCD
% 4: spatial multiplexing using antenna ports with UE-specific reference 
%    signals
% 

% Inputs:      
% in            - 2D array of 'ap' antenna ports x 'Ma' symbols per ap
%
% v             - Number of layers
%
% p             - antenna port vector: is checked here, but not modified
%
% style         - single antenna port (0), ...without CCD (1), ...large 
%               delay CCD (2), UE-specific ref. signals (3), transmit 
%               diverssity (4)
%
% Outputs:
% out           - Complex valued modulation symbols, 2D array of v layers 
%               x Ml symbols per layer (input of Layer Demapper)
%
% Spec:         (3GPP TS 36.211 section 6.3.4 v10.5.0)
% Notes:        Only 'single-antenna port', 'transmit diversity' and 
%               'Precoding for spatial multiplexing using antenna ports 
%               with UE-specific reference signals' currently supported
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last revision:1.1.2013 
% 

% Precoding of the complex-valued modulation symbols of each layer for 
% transmission on selected antenna ports p
% Valid combinations...
% ...single antenna port:
% p=0,4,5,7, or 8
%
% ...spatial multiplexing:
% p=[0,1] or p=[15,16] for CSI reporting (v=2); 
% p=[0,1,2,3] or p=[15,16,17,18] for CSI reporting (p=4)
% 
% ...transmit diversity: 
% p=[0,1] (v=2) or p=[0,1,2,3] (v=4)
function out = lte_PDSCH_unprecoding(in, v, style)
% Ms0 is the number of symbols on condeword 0. It is needed to calculate
% the number of output samples for tx_diversity when v=4

    % styles:
    single_ap = 0;
    tx_div = 1;     % transmit diversity
    sm_without_CCD = 2;
    sm_CCD = 3;
    sm_UE = 4;  % spatial multiplexing using antenna ports with UE-specific
    % reference signals

%     in
%     p

    [ap, Ma] = size(in); % v_max may be 8 or the actual number of layers (v),
    % depending on the implementation of the Layer Mapper processing block

    if (Ma == 0)
        out = [];
        return;
    end
    
    out = 0;
    
    % initialize()
    % out = zeros(8,4*Ml);  % up to 8 antenna ports and 4 times the number of
    % input symbols per layer on each antenna port (tx_diversity)
    
    % Transpose of precoding matrix for transmit diversity and 2 antenna ports: W_td_2 * W_td2T = eye(4)
    W_td_2T = 1/sqrt(2)*[1 0 j 0;
                        0 -1 0 j;
                        0 1 0 j;
                        1 0 -j 0]'; % Notice the transpose sign (')
    
    % Transpose of precoding matrix for transmit diversity and 4 antenna ports
    W_td_4T = 1/sqrt(2)*[1 0 0 0 j 0 0 0;
                        0 0 0 0 0 0 0 0;
                        0 -1 0 0 0 j 0 0;
                        0 0 0 0 0 0 0 0;
                        0 1 0 0 0 j 0 0;
                        0 0 0 0 0 0 0 0;
                        1 0 0 0 -j 0 0 0;
                        0 0 0 0 0 0 0 0;
                        0 0 0 0 0 0 0 0;
                        0 0 1 0 0 0 j 0;
                        0 0 0 0 0 0 0 0;
                        0 0 0 -1 0 0 0 j;
                        0 0 0 0 0 0 0 0;
                        0 0 0 1 0 0 0 j;
                        0 0 0 0 0 0 0 0;
                        0 0 1 0 0 0 -j 0]'; % Notice the transpose sign (')
    
    if (style == single_ap)     % Single antenna port (v=1, not checked)
        % 36.211 Section 6.3.4.1 v10.5.0
        %if ((length(p)==1) && ((p==0) || (p==4) || (p==5) || (p==7) || (p==8)))
        if (ap==1) % Just check that the number of antenna ports is correct
            Ml = Ma;
            out = zeros(1,Ml);
            for (i=0:Ml-1)
                out(1, i+1) = in(1,i+1); % one layer assigned to first row of output matrix, irrespective of antenna port
            end
        else
            fprintf('\nERROR: Specified antenna port not valid for single antenna port mode (v must be 1).');
        end
        
    elseif (style == tx_div)    % Transmit Diversity: v = 2 or 4
        % 36.211 Section 6.3.4.3 v10.5.0
        if (v==2)
            %if ((length(p)==2) && (p(1)==0) && (p(2)==1))
            if (ap==2)
                Ml = floor(Ma/2); % number of symbols per antenna port
                if (2*Ml < Ma)
                    fprintf('\n ERROR: Un-Precoding (Transmit Diversity) - ap = 2 antenna ports requires the number of samples on each antenna port being integer divisible by 2');
                end
                out = zeros(2,Ml);
                i=0:(Ml-1);
%                 [in(1,2*i+1); in(2,2*i+1); in(1,2*i+1+1); in(2,2*i+1+1)]
%                 out(1,i+1)
                r_out(1,i+1) = W_td_2T(1,:)*[in(1,2*i+1); in(2,2*i+1); in(1,2*i+1+1); in(2,2*i+1+1)];
                r_out(2,i+1) = W_td_2T(2,:)*[in(1,2*i+1); in(2,2*i+1); in(1,2*i+1+1); in(2,2*i+1+1)];
                i_out(1,i+1) = W_td_2T(3,:)*[in(1,2*i+1); in(2,2*i+1); in(1,2*i+1+1); in(2,2*i+1+1)];
                i_out(2,i+1) = W_td_2T(4,:)*[in(1,2*i+1); in(2,2*i+1); in(1,2*i+1+1); in(2,2*i+1+1)];
                out = r_out + i_out*j;
            else
                ap
                fprintf('\nERROR: Un-Precoding (Transmit Diversity) - Wrong number of antenna ports (ap) for v=2 layers.\n');
                fprintf('ap should be 2 as well.\n');
                return;
            end
            
        elseif (v==4)
            %if ((length(p)==4) && (p(1)==0) && (p(2)==1) && (p(3)==2) && (p(4)==3))
            if (ap==4)
                Ml = floor(Ma/4); % there is questionable dependency--here ignored--for defining Ma on p. 62 of 36.211 Section 6.3.4.3 v10.5.0
                if (4*Ml < Ma)
                    fprintf('\nERROR: Un-Precoding (Transmit Diversity) - ap = 4 antenna ports requires the number of samples on each antenna port being integer divisible by 4');
                end
                out = zeros(4,Ml);
                i=0:(Ml-1);
                r_out(1,i+1) = W_td_4T(1,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                r_out(2,i+1) = W_td_4T(2,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                r_out(3,i+1) = W_td_4T(3,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                r_out(4,i+1) = W_td_4T(4,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                i_out(1,i+1) = W_td_4T(5,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                i_out(2,i+1) = W_td_4T(6,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                i_out(3,i+1) = W_td_4T(7,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                i_out(4,i+1) = W_td_4T(8,:)*[in(1,4*i+1); in(2,4*i+1); in(3,4*i+1); in(4,4*i+1); in(1,4*i+1+1); in(2,4*i+1+1); in(3,4*i+1+1); in(4,4*i+1+1); in(1,4*i+2+1); in(2,4*i+2+1); in(3,4*i+2+1); in(4,4*i+2+1); in(1,4*i+3+1); in(2,4*i+3+1); in(3,4*i+3+1); in(4,4*i+3+1)];
                out = r_out + i_out*j;
            else
                ap
                fprintf('ERROR: Un-Precoding (Transmit Diversity) - Wrong number of antenna ports (ap) for v=4 layers.\n');
                fprintf('ap should be 4 as well.\n');
                return;
            end
        else
            fprintf('ERROR: Transmit Diversity - Wrong antenna ports or number of layers chosen.\n');
            return;
        end
        
    elseif (style == sm_without_CCD)    % spatial multiplexing without CCD
        % 36.211 Section 6.3.4.2.1 v10.5.0
        fprintf('\nNOT YET IMPLEMENTED\n.');
        
    elseif (style == sm_CCD)    % spatial multiplexing without CCD
        % 36.211 Section 6.3.4.2.2 v10.5.0
        fprintf('\nNOT YET IMPLEMENTED\n.');

    elseif (style == sm_UE)    % spatial multiplexing with UE-specific reference signals
        % 36.211 Section 6.3.4.4 v10.5.0
        if ((v>=1) && (v<=8) && (ap==v))
            Ml = Ma;
            %out = zeros(v,Ma);
%             for (i=1:v)
%                 if (p(i)~=(6+i))
%                     fprintf('\nERROR: Specified antenna ports not valid for the "spatial multiplexing with UE-specific reference signals" transmission mode.');
%                     return;
%                 end
%             end
            out = in; % mapped to the first v rows of output matrix, despite the antenna ports being 7, 8, ...
        else
            fprintf('\nERROR: Number of layers and antenna ports should be between 1 and 8 for the "spatial multiplexing with UE-specific reference signals" transmission mode.');
        end

    else
        fprintf('\nERROR: Unknowns transmission style. Should be either single antenna port (0), spatial multiplexing diversity (1) or spatial multiplexing (2)');
    end
end

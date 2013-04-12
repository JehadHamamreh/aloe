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

% Precoder: Takes a block of vectors containing complex-valued modulation 
% symbols to generate block of vectors to be mapped to resources on each of
% the antenna ports
% - Previous module: Layer Mapping
% - Next module: Resource Mapping
%
% Downlink (DL) Transmitter (PDSCH)
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
% in            - Complex valued modulation symbols, 2D array of v layers 
%               x Ml symbols per layer (in = out of layer mapper)
%
% v             - Number of layers. Could be obtained from in, but input v
%               reduces complexity and increases flexibility
%
% p             - antenna port vector: is checked here, but not modified
%
% style         - single antenna port (0), ...without CCD (1), ...large 
%               delay CCD (2), UE-specific ref. signals (3), transmit 
%               diverssity (4)
%
% Outputs:
% out           - 2D array of 'ap' antenna ports x 'Ml' symbols per ap

% Spec:         3GPP TS 36.211 section 6.3.4 v10.5.0
% Notes:        Only 'single-antenna port', 'transmit diversity' and 
%               'Precoding for spatial multiplexing using antenna ports 
%               with UE-specific reference signals' currently supported
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last revision:1.1.2013 

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
function out = lte_PDSCH_precoding(in, p, style)
% Ms0 is the number of symbols on condeword 0. It is needed to calculate
% the number of output samples for tx_diversity when v=4

    % styles:
    % Precoding for transmission on a single antenna port (Secction 6.3.4.1)
    single_ap = 0;
    
    % Precoding for transmit diversity (Section 6.3.4.3)
    tx_div = 1;     
    
    % Precoding for spatial multiplexing using antenna ports with cell-specific
    % reference signals (Section 6.3.4.2) -- NOT YET SUPPORTED
    sm_without_CCD = 2; % Precoding without CDD (Section 6.3.4.2.1)
    sm_CCD = 3;         % Precoding for large delay CDD (Section 6.3.4.2.2)
    
    % Precoding for spatial multiplexing using antenna ports with UE-specific
    % reference signals: 
    sm_UE = 4;          % (Section 6.3.4.4)

%     in
%     p
    
    [v, Ml] = size(in);
    if (Ml == 0)
        out = [];
        return;
    end
    
    out = 0;
    
    % Precoding matrix for transmit diversity and 2 antenna ports
    W_td_2 = 1/sqrt(2)*[1 0 j 0;
                        0 -1 0 j;
                        0 1 0 j;
                        1 0 -j 0];  
    
    % Precoding matrix for transmit diversity and 4 antenna ports
    W_td_4 = 1/sqrt(2)*[1 0 0 0 j 0 0 0;
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
                        0 0 1 0 0 0 -j 0];
    
    if (style == single_ap)     % Single antenna port (v=1, not checked)
        % 36.211 Section 6.3.4.1 v10.5.0
        if ((length(p)==1) && ((p==0) || (p==4) || (p==5) || (p==7) || (p==8)))
            Ma = Ml;
            out = zeros(1,Ma);
            for (i=0:Ma-1)
                out(i+1) = in(1,i+1); % one layer assigned to first row of output matrix, irrespective of antenna port
            end
        else
            fprintf('\nERROR: Specified antenna port not valid for single antenna port mode (v must be 1).');
        end
        
    elseif (style == tx_div)    % Transmit Diversity: v = 2 or 4
        fprintf('Precoding for transmit diversity.\n');
        % 36.211 Section 6.3.4.3 v10.5.0
        if (v==2)
            if ((length(p)==2) && (p(1)==0) && (p(2)==1))
                Ma = 2*Ml; % number of symbols per antenna port
                out = zeros(2,Ma);
                i=0:(Ml-1);
                out(1,2*i+1)    = W_td_2(1,:)*[real(in(1,i+1)); real(in(2,i+1)); imag(in(1,i+1)); imag(in(2,i+1))];
                out(2,2*i+1)    = W_td_2(2,:)*[real(in(1,i+1)); real(in(2,i+1)); imag(in(1,i+1)); imag(in(2,i+1))];
                out(1,2*i+1+1)  = W_td_2(3,:)*[real(in(1,i+1)); real(in(2,i+1)); imag(in(1,i+1)); imag(in(2,i+1))];
                out(2,2*i+1+1)  = W_td_2(4,:)*[real(in(1,i+1)); real(in(2,i+1)); imag(in(1,i+1)); imag(in(2,i+1))];
            else
                fprintf('ERROR: Transmit Diversity - Wrong antenna ports chosen.\n');
                return;
            end    
            
        elseif (v==4)
            if ((length(p)==4) && (p(1)==0) && (p(2)==1) && (p(3)==2) && (p(4)==3))
                Ma = 4*Ml; % there is questionable dependency--here ignored--for defining Ma on p. 62 of 36.211 Section 6.3.4.3 v10.5.0
                out = zeros(4,Ma);
                i=0:(Ml-1);
                out(1,4*i+1)    = W_td_4(1,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(2,4*i+1)    = W_td_4(2,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(3,4*i+1)    = W_td_4(3,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(4,4*i+1)    = W_td_4(4,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(1,4*i+1+1)  = W_td_4(5,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(2,4*i+1+1)  = W_td_4(6,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(3,4*i+1+1)  = W_td_4(7,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(4,4*i+1+1)  = W_td_4(8,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(1,4*i+2+1)  = W_td_4(9,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(2,4*i+2+1)  = W_td_4(10,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(3,4*i+2+1)  = W_td_4(11,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(4,4*i+2+1)  = W_td_4(12,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(1,4*i+3+1)  = W_td_4(13,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(2,4*i+3+1)  = W_td_4(14,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(3,4*i+3+1)  = W_td_4(15,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
                out(4,4*i+3+1)  = W_td_4(16,:)*[real(in(1,i+1)); real(in(2,i+1)); real(in(3,i+1)); real(in(4,i+1)); imag(in(1,i+1)); imag(in(2,i+1)); imag(in(3,i+1)); imag(in(4,i+1))];
            else
                fprintf('ERROR: Transmit Diversity - Wrong antenna ports chosen.\n');
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
        fprintf('Precoding for spatial multiplexing using antenna ports with UE-specific reference signals.\n');
        if ((v>=1) && (v<=8) && (length(p)==v))
            Ma = Ml;
            out = zeros(v,Ma);
            for i=1:v
                if (p(i)~=(6+i))
                    fprintf('\nERROR: Specified antenna ports not valid for the "spatial multiplexing with UE-specific reference signals" transmission mode.');
                    return;
                end
            end
            out = in; % mapped to the first v rows of output matrix, despite the antenna port indexes 7, 8, 9, ...
        else
            fprintf('\nERROR: Number of layers and antenna ports should be between 1 and 8 for the "spatial multiplexing with UE-specific reference signals" transmission mode.');
        end

    else
        fprintf('\nERROR: Unknowns transmission style. Should be either single antenna port (0), spatial multiplexing diversity (1) or spatial multiplexing (2)');
    end
end

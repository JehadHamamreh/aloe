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

% Layer Demapper: Maps complex-valued symbols arriving on one or several 
% layers to one or two codewords
% - Previous module: Unprecoding
% - Next module: Modulation Demapping
%
% Downlink (DL) Receiver, PDSCH
% Different modes: single antenna port, transmit diversity, spatial multiplexing, 

% Inputs:
% in            - 2D array of v layers x Ml symbols per layer
%               v: number of layers
%               Ml: symbols per layer (equal for all layers)
%               
% q             - Number of codewords: q = 1, 2
%
% v             - Number of layers: v = 1, 2, 4, 8 (a function of style)
%
% style         - single antenna port (0), transmit diversity (1), or
%               spatial multiplexing (2) 
%
% Outputs:     
% out0, out1    - 1D vectors of Ms0 and Ms1 elements, representing complex
%               valued symbols for modulation demapping at the receiver
%               out0: data on codeword 0
%               out1: data on codeword 1, exists (non-zero) if q=2

% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Spec:         3GPP TS 36.211 section 6.3.3 v10.5.0
% Notes:        -
% Author:       Vuk Marojevic (vuk.marojevic@gmail.com)
% Last Revision:1.1.2013
% 

% Isma, si cambiamos el tamaño del time slot (a la brava), podría pasar
% que el numero de muestras no es divisible por 2, 3, o 4 según el caso.
% Esto puede resultar en perdidas de muestras o que no haya el mismo numero
% de muestras en todas las salida.
% Lo qe hago es chequear si es divisible y si devuelvo un error message.

% Mapping of complex valued symbols on 1 or 2 codewords (q) to up to 8
% layers (v)
% Valid combinations...
% ...single antenna port:
% v=1, q=1
% ...spatial multiplexing:
% v=2, q=2
% v=3, q=1 
% v=3, q=2 
% v=4, q=1 
% v=4, q=2
% v=5, q=2 
% v=6, q=2 
% v=7, q=2 
% v=8, q=2
% ...transmit diversity:
% v=2, q=1 
% v=4, q=1
function [out0, out1] = lte_PDSCH_layer_demapper(in, v, q, style)

    % symbolic constants for MIMO styles:
    single_ap = 0;
    tx_div = 1;
    sp_mux = 2;
    
    % check/obtain parameters
    [vx, Ml] = size(in);
    
    if (Ml == 0)
        % Return empty cell arrays (cell array of 0x0)
        out0 = {};
        out1 = {};
        return;
    end
    
    if (vx ~= v)
        fprintf('\nERROR: Number of rows of input array does not coincide with the specified number of layers.\n');
        return;
    end
    
    % DSP
    if (style == single_ap)     % Single antenna port
        % (36.211 Section 6.3.3.1 v10.5.0)
        Ms0 = Ml;
        if ((v>1) || (q>1))
            fprintf('\nWarning: Single antenna port uses v=1 layer and q=1 codeword. Ignoring specified v and q.');
        end
        q = 1; v = 1;   % one codeword and one layer if single antenna port
        %out0 = zeros(1,Ms0);
        out0 = in;
        out1 = 0;          
 
    elseif (style == tx_div)    % TRANSMIT DIVERSITY: 1 codeword; v = 2 or 4 (v=ap--number of antenna ports)
         % (36.211 Section 6.3.3.3 v10.5.0)
        if ((v==2) && (q==1))
            Ms0 = 2*Ml;
            out0 = zeros(q,Ms0);
            out1 = 0;
            for i=0:(Ml-1)
                out0(2*i+1)     = in(1, i+1);      % even symbols
                out0(2*i+1+1)   = in(2, i+1);    % odd symbools
            end
            
            
        elseif ((v==4) && (q==1))
            Ms0 = 4*Ml;
            fprintf('\nWarning: Two null symbols might have been appended by the layer_mapper, which should be removed here');
            out0 = zeros(1,Ms0);
            out1 = 0;
            for i=0:(Ml-1)
                out0(4*i+1)     = in(1, i+1);
                out0(4*i+1+1)   = in(2, i+1);
                out0(4*i+2+1)   = in(3, i+1);
                out0(4*i+3+1)   = in(4, i+1);
            end
            
        else
            fprintf('ERROR: Transmit Diversity - Invalid combination of layers v and codewords.\n');
            return;
        end
        
    elseif (style == sp_mux)    % Spatial Multiplexing: v <= # antenna ports
        % The case of a single codeword mapped to multiple layers is only 
        % applicable when the number of cell-specific reference signals is 
        % four or when the number of UE-specific reference signals is two or larger.

        % (36.211 Section 6.3.3.2 v10.5.0)
        if ((v==1) && (q==1))   % este modo no sé que tiene de spatial multiplexing
            Ms0 = Ml;
            %out0 = zeros(1,Ms0);
            out0 = in;
            out1 = 0;
                
        elseif (v==2)
            if (q==1)
                Ms0 = 2*Ml;
                out0 = zeros(1,Ms0);
                out1 = 0;
                for i=0:(Ml-1)
                    out0(2*i+1)      = in(1, i+1);   % layer 1 -> even-numbered symbols
                    out0(2*i+1+1)    = in(2, i+1);   % layer 2 -> odd-numbered symbools
                end
            else % q=2
                Ms0 = Ml; 
                Ms1 = Ml;
                out0 = zeros(1,Ms0);
                out1 = zeros(1,Ms1);
                for i=0:(Ml-1)
                    out0(i+1) = in(1, i+1);     % layer 1 -> codeword 1
                    out1(i+1) = in(2, i+1);     % layer 2 -> codeword 2
                end
            end
            
        elseif (v==3)
            if (q==1)
                Ms0 = 3*Ml;
                %Ms1 = 0;
                out0 = zeros(1,Ms0);
                out1 = 0;
                for i=0:(Ml-1)
                    out0(3*i+1)     = in(1, i+1);   % layer 1 -> codeword 1
                    out0(3*i+1+1)   = in(2, i+1);   % layer 2 -> codeword 1
                    out0(3*i+2+1)   = in(3, i+1);   % layer 3 -> codeword 1
                end
                
            else % q==2 
                Ms0 = Ml; 
                Ms1 = 2*Ml;
                out0 = zeros(1,Ms0);
                out1 = zeros(1,Ms1);
                for i=0:(Ml-1)
                    out0(i+1)       = in(1, i+1);   % layer 1 -> codeword 1 
                    out1(2*i+1)     = in(2, i+1);   % layer 2 -> codeword 2, even-numbered symbols
                    out1(2*i+1+1)   = in(3, i+1);   % layer 3 -> codeword 2, odd-numbered symbols
                end
            end

        elseif (v==4)
            if (q==1)
                Ms0 = 4*Ml;
                %Ms1 = 0;
                out0 = zeros(1,Ms0);
                out1 = 0;
                for i=0:(Ml-1)
                    out0(4*i+1)     = in(1, i+1);   % layer 1 -> codeword 1 
                    out0(4*i+1+1)   = in(2, i+1);   % layer 2 -> codeword 1
                    out0(4*i+2+1)   = in(3, i+1);	% layer 3 -> codeword 1
                    out0(4*i+3+1)   = in(4, i+1);	% layer 4 -> codeword 1
                end
                
            else % q=2
                Ms0 = 2*Ml;
                Ms1 = 2*Ml;
                out0 = zeros(1,Ms0);
                out1 = zeros(1,Ms1);
                for i=0:(Ml-1)                    
                    out0(2*i+1)     = in(1, i+1);   % layer 1 -> codeword 1, even-numbered symbols
                    out0(2*i+1+1)   = in(2, i+1);   % layer 2 -> codeword 1 odd-numbered symbols
                    out1(2*i+1)     = in(3, i+1);   % layer 3 -> codeword 2 even-numbered symbols
                    out1(2*i+1+1)   = in(4, i+1);   % layer 4 -> codeword 2 odd-numbered symbols
                end                
            end
            
        elseif ((v==5) && (q==2))  % q = 2!
            Ms0 = 2*Ml; 
            Ms1 = 3*Ml;
            out0 = zeros(1,Ms0);
            out1 = zeros(1,Ms1);
            for i=0:(Ml-1)
                out0(2*i+1)     = in(1, i+1);
                out0(2*i+1+1)   = in(2, i+1);
                out1(3*i+1)     = in(3, i+1);
                out1(3*i+1+1)   = in(4, i+1);
                out1(3*i+2+1)   = in(5, i+1);
            end
            
        elseif ((v==6) && (q==2)) % q = 2!
            Ms0 = 3*Ml;
            Ms1 = 3*Ml;
            out0 = zeros(1,Ms0);
            out1 = zeros(1,Ms1);
            for i=0:Ml-1
                out0(3*i+1)     = in(1, i+1);
                out0(3*i+1+1)   = in(2, i+1);
                out0(3*i+2+1)   = in(3, i+1);
                out1(3*i+1)     = in(4, i+1);
                out1(3*i+1+1)   = in(5, i+1);
                out1(3*i+2+1)   = in(6, i+1);
            end
                
        elseif ((v==7) && (q == 2)) % q = 2!
             Ms0 = 3*Ml;
             Ms1 = 4*Ml;
             out0 = zeros(1,Ms0);
             out1 = zeros(1,Ms1);
             for i=0:(Ml-1)
                 out0(3*i+1)    = in(1, i+1);
                 out0(3*i+1+1)  = in(2, i+1);
                 out0(3*i+2+1)  = in(3, i+1);
                 out1(4*i+1)    = in(4, i+1);
                 out1(4*i+1+1)  = in(5, i+1);
                 out1(4*i+2+1)  = in(6, i+1);
                 out1(4*i+3+1)  = in(7, i+1);
            end
                
        elseif ((v==8) && (q == 2))   % q = 2!
            Ms0 = 4*Ml;
            Ms1 = 4*Ml;
            out0 = zeros(1,Ms0);
            out1 = zeros(1,Ms1);
            for i=0:(Ml-1)
                out0(4*i+1)     = in(1, i+1);
                out0(4*i+1+1)   = in(2, i+1);
                out0(4*i+2+1)   = in(3, i+1);
                out0(4*i+3+1)   = in(4, i+1);
                out1(4*i+1)     = in(5, i+1);
                out1(4*i+1+1)   = in(6, i+1);
                out1(4*i+2+1)   = in(7, i+1);
                out1(4*i+3+1)   = in(8, i+1);
            end
                
        else
            fprintf('\nERROR: Spatial Multiplexing - Invalid combinarion of number or layers v and codewords q');
        end
    else
        fprintf('\nERROR: Unknowns transmission style. Should be either single antenna port (0), transmit diversity (1) or spatial multiplexing (2)');
    end 
end

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

% Layer Mapper: Maps complex-valued modulation symbols from one or two 
% codewords to one or several layers
% - Previous module: Modulation Mapping
% - Next module: Precoding

% Downlink (DL) Transmitter, PDSCH
% Different modes: single antenna port, transmit diversity, spatial multiplexing, 

% Inputs:      
% in0, in1      - Complex valued modulation symbols, 1D vectors
%               of Ms0 and Ms1 elements
%               in0: data on codeword 0
%               in1: data on codeword 1, exists (non-zero) if q=2 codewords
%               Ms1, Ms2: symbols per codeword, as a function of the time slot
%
% q             - Number of codewords: q = 1, 2
%
% v             - Number of layers: v = 1, 2, 4, 8 (a function of style)
%
% style         - single antenna port (0), transmit diversity (1), or
%               spatial multiplexing (2) 
%
% Outputs:     
% out           - 2D array of v layers x Ml symbols per layer
%               v: number of layers
%               Ml: symbols per layer (equal for all layers)

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
function out = lte_PDSCH_layer_mapper(in0, in1, v, q, style)

    % styles:
    single_ap = 0;
    tx_div = 1;
    sp_mux = 2;
    
%    fprintf('\nCodewords:\n');
%     in0 % debugging: see input symbols, codeword 0
%     in1 % debugging: see input symbols, codeword 1
    Ms0 = length(in0);
    Ms1 = length(in1);
    
    if (Ms0 == 0)
        out = [];
        return;
    end
        
    % initialize()
    % out = zeros(8,Ms0);    % allocated maximum size, impractical
    % esto es cutre porque en ningún caso se usarán 8xMs0
    % elementos del array. Varía entre 1xMs0 y 8x(Ms0/4)
    
    if (style == single_ap)     % Single antenna port
        % 36.211 Section 6.3.3.1 v10.5.0
        Ml = Ms0;
        if ((v>1) || (q>1))
            fprintf('\nWarning: Single antenna port uses v=1 layer and q=1 codeword. Ignoring specified v and q.');
        end
        q = 1; v = 1;   % one codeword and one layer if single antenna port
        out = zeros(1,Ml);
        for (i=0:Ml-1)
            out(1, i+1) = in0(i+1);
        end
           
    elseif (style == tx_div)    % Transmit Diversity: 1 codeword; v = # antenna ports
         % 36.211 Section 6.3.3.3 v10.5.0
        if ((v==2) && (q==1))
            Ml = floor(Ms0/2); 
            if ((2*Ml) < Ms0) % check that Ms mod 2 = 0 and report an error otherwise
                fprintf('ERROR: Number of samples not integer divisible by 2.\n');
                return;
            end
            out = zeros(2,Ml);
            for (i=0:Ml-1)
                out(1, i+1) = in0(2*i+1);      % even symbols
                out(2, i+1) = in0(2*i+1+1);    % odd symbools
            end
            
        elseif ((v==4) && (q==1))
            if (mod(Ms0, 4) > 0)   
                Ms0 = Ms0+2;        % Append 2 null symbols to input
                in0(Ms0-1) = 0;
                in0(Ms0) = 0;
            end
            Ml = floor(Ms0/4);
            if ((4*Ml) < Ms0)
                fprintf('ERROR: Number of samples not integer divisible by 4.\n');
                return;
            end
            out = zeros(4,Ml);
            for (i=0:Ml-1)
                out(1, i+1) = in0(4*i+1);
                out(2, i+1) = in0(4*i+1+1);
                out(3, i+1) = in0(4*i+2+1);
                out(4, i+1) = in0(4*i+3+1);
            end
            
        else
            fprintf('ERROR: Transmit Diversity - Invalid combination of layers v and codewords.\n');
            return;
%           errormsg("\nWrong combination of layers %d and codewords %d.", v, q)
%           errormsg("v = 2 or 4 and q = 1 for transmit diversity.");
%           return -1;
        end
        
    elseif (style == sp_mux)    % Spatial Multiplexing: v <= # antenna ports
        % The case of a single codeword mapped to multiple layers is only 
        % applicable when the number of cell-specific reference signals is 
        % four or when the number of UE-specific reference signals is two or larger.

        % 36.211 Section 6.3.3.2 v10.5.0
        if ((v==1) && (q==1))   % defined by 3GPP, but is not spatial multiplexing
            Ml = Ms0;
            out = zeros(1,Ml);
            for(i=0:Ml-1)
                out(1, i+1) = in0(i+1);
            end
                
        elseif (v==2)
            if (q==1)
                Ml = floor(Ms0/2);
                if ((2*Ml) < Ms0)
                    fprintf('ERROR: Number of samples not integer divisible by 2.\n');
                    return;
                end
                out = zeros(2,Ml);
                for (i=0:Ml-1)
                    out(1, i+1) = in0(2*i+1);     % even symbols
                    out(2, i+1) = in0(2*i+1+1);   % odd symbools
                end
            else
                Ml = Ms0;
                if (Ms1 ~= Ms0)
                    fprintf('ERROR: Number of samples on both codewords not the same.\n');
                    return;
                end
                out = zeros(2,Ml);
                for (i=0:Ml-1)
                    out(1, i+1) = in0(i+1);     % codeword 1 -> layer 1
                    out(2, i+1) = in1(i+1);     % codeword 2 -> layer 2
                end
            end
            
        elseif (v==3)
            if (q==1)
                Ml = floor(Ms0/3);
                if ((3*Ml) < Ms0)
                    fprintf('ERROR: Number of samples not integer divisible by 2.\n');
                    return;
                end
                out = zeros(3,Ml);
                for (i=0:Ml-1)
                    out(1, i+1) = in0(3*i+1);     % codeword 1 -> layer 1
                    out(2, i+1) = in0(3*i+1+1);   % codeword 1 -> layer 2
                    out(3, i+1) = in0(3*i+2+1);   % codeword 1 -> layer 3
                end
                
            else 
                Ml = Ms0; % = Ms1/2
                if (Ms1==(2*Ms0))
                    out = zeros(3,Ml);
                    for (i=0:Ml-1)
                        out(1, i+1) = in0(i+1);     % codeword 1 -> layer 1
                        out(2, i+1) = in1(2*i+1);   % codeword 2 even -> layer 2
                        out(3, i+1) = in1(2*i+1+1); % codeword 2 odd -> layer 3
                    end
                else
                    fprintf('ERROR: Codeword 2 should contain twice as many samples as codeword 1.\n');
                    return;
                end
                    
            end
                
        elseif (v==4)
            if (q==1)
                Ml = floor(Ms0/4); % integer division
                if ((4*Ml) < Ms0)
                    fprintf('ERROR: Number of samples not integer divisible by 4.\n');
                    return;
                end
                out = zeros(4,Ml);
                for (i=0:Ml-1)
                    out(1, i+1) = in0(4*i+1);   % codeword 1 -> layer 1
                    out(2, i+1) = in0(4*i+1+1); % codeword 1 -> layer 2
                    out(3, i+1) = in0(4*i+2+1);	% codeword 1 -> layer 3
                    out(4, i+1) = in0(4*i+3+1);	% codeword 1 -> layer 4
                end
                
            else
                Ml = Ms0/2;
                if (((2*Ml) < Ms0) || (Ms1 ~= Ms0))
                    fprintf('ERROR: Number of samples on both codewords not the same or not integer divisible by 2.\n');
                    return;
                end
                out = zeros(4,Ml);
                for (i=0:Ml-1)                    
                    out(1, i+1) = in0(2*i+1);     % codeword 1 even -> layer 1
                    out(2, i+1) = in0(2*i+1+1);   % codeword 1 odd -> layer 2
                    out(3, i+1) = in1(2*i+1);     % codeword 2 even -> layer 3
                    out(4, i+1) = in1(2*i+1+1);   % codeword 2 odd -> layer 4
                end                
            end
            
        elseif ((v==5) && (q==2))  % q = 2!
            Ml = Ms0/2; % = Ms1/3
            if (((2*Ml) < Ms0) || ((2*Ms1) ~= 3*Ms0))
                fprintf('\n ERROR: Spatial Multiplexing - v = 5 layers requires q = 2 codewords and 3/2 as many samples on codeword 2 as on codeword 1');
                return;
            end
            out = zeros(5,Ml);
            for (i=0:Ml-1)
                out(1, i+1) = in0(2*i+1);
                out(2, i+1) = in0(2*i+1+1);
                out(3, i+1) = in1(3*i+1);
                out(4, i+1) = in1(3*i+1+1);
                out(5, i+1) = in1(3*i+2+1);
            end
            
        elseif ((v==6) && (q==2)) % q = 2!
            Ml = floor(Ms0/3);
            if (((3*Ml) < Ms0) || (Ms1 ~= Ms0))
                fprintf('\n ERROR: Spatial Multiplexing - v = 6 layers requires q = 2 codewords with the same number of samples, integer divisible by 3');
                return;
            end
            out = zeros(6,Ml);
            for i=0:Ml-1
                out(1, i+1) = in0(3*i+1);
                out(2, i+1) = in0(3*i+1+1);
                out(3, i+1) = in0(3*i+2+1);
                out(4, i+1) = in1(3*i+1);
                out(5, i+1) = in1(3*i+1+1);
                out(6, i+1) = in1(3*i+2+1);
            end

        elseif ((v==7) && (q==2)) % q = 2!
             Ml = floor(Ms0/3); % = Ms1/4
             if (((3*Ml) < Ms0) || ((4*Ms0) ~= (3*Ms1)))
                fprintf('\n ERROR: Spatial Multiplexing - v = 7 layers requires q = 2 codewords and 4/3 samples on codeword 2 w.r.t codeword 1');
                return;
             end
             out = zeros(7,Ml);
             for (i=0:Ml-1)
                 out(1, i+1) = in0(3*i+1);
                 out(2, i+1) = in0(3*i+1+1);
                 out(3, i+1) = in0(3*i+2+1);
                 out(4, i+1) = in1(4*i+1);
                 out(5, i+1) = in1(4*i+1+1);
                 out(6, i+1) = in1(4*i+2+1);
                 out(7, i+1) = in1(4*i+3+1);
            end
                
        elseif ((v==8) && (q == 2))   % q = 2
            Ml = floor(Ms0/4);
            if (((4*Ml) < Ms0) || (Ms0 ~= Ms1))
                fprintf('\n ERROR: Spatial Multiplexing - v = 8 layers requires q = 2 codewords with the same number of samples, integer divisible by 4');
                return;
            end
            out = zeros(8,Ml);
            for i=0:Ml-1
                out(1, i+1) = in0(4*i+1);
                out(2, i+1) = in0(4*i+1+1);
                out(3, i+1) = in0(4*i+2+1);
                out(4, i+1) = in0(4*i+3+1);
                out(5, i+1) = in1(4*i+1);
                out(6, i+1) = in1(4*i+1+1);
                out(7, i+1) = in1(4*i+2+1);
                out(8, i+1) = in1(4*i+3+1);
            end
                
        else
            fprintf('\n ERROR: Spatial Multiplexing - Invalid combinarion of number or layers v and codewords q');
%             errormsg("Wrong number of layers %d"., v)
%             errormsg("Should be either 2 or 4 for transmit diversity mode".);
%             return -1;        
        end
    else
        fprintf('\nERROR: Unknowns transmission style. Should be either single antenna port (0), transmit diversity (1) or spatial multiplexing (2)');
    end
end

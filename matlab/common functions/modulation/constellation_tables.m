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

% Author: 	Vuk Marojevic (vuk.marojevic@gmail.com)
% Last revision:1.1.2013 

% Creates constallation tables and vectors for soft demapping
% ===========================================================
% INPUTS
% debug:        set to 1 when testing against Matlab demodulators
% matlab_constellation:    set to 1 when testing against Matlab
% demodulators, using modem.qamdemod()
%
% OUTPUTS
% tables and vectors used for soft and hard demapping for BPSK, QPSK,
% 16QAM, and 64 QAM modulated symbols

function [bpsk_table, S0_bpsk, S1_bpsk, qpsk_table, S0_qpsk, S1_qpsk, qam16_table, S0_qam16, S1_qam16, qam64_table, S0_qam64, S1_qam64] = constellation_tables(matlab_constellation)

    % LTE-BPSK constellation:
	%    Q
	%    |  0
	%---------> I
	% 1  |
	
    D = sqrt(2);
    BPSK_LEVEL = 1/D;   % 3GPP: TS 136 211 - V 10.5.0
    
    % BPSK modulation table
    bpsk_table = zeros(2,1);
    bpsk_table(1) = BPSK_LEVEL + BPSK_LEVEL*1i;
	bpsk_table(2) = -BPSK_LEVEL -BPSK_LEVEL*1i;

    % BSPK symbols containing a '0' and a '1' (only two symbols, 1 bit)
    S0_bpsk = zeros(1,1);
    S0_bpsk(1,1) = 1;
  
    S1_bpsk = zeros(1,1);
    S1_bpsk(1,1) = 2;


    %%
    % LTE-QPSK constellation:
	%     Q
	% 10  |  00
	%-----------> I
	% 11  |  01

%     if matlab_constellation
%         D = 1;            % Testing: Matlab function outcomes assume QPSK level 1, rather than 1/sqrt(2)
%     else
%         D = sqrt(2);      % 3GPP: TS 136 211 - V 10.5.0
%     end
    QPSK_LEVEL = 1/D;       % D = sqrt(2) OK, since applying modem.genqamdemod() with LTE-specific costellation
    
    % QPSK modulation table
    qpsk_table = zeros(4,1);
    qpsk_table(1) = QPSK_LEVEL + QPSK_LEVEL*1i;
	qpsk_table(2) = QPSK_LEVEL - QPSK_LEVEL*1i;
	qpsk_table(3) = -QPSK_LEVEL + QPSK_LEVEL*1i;
	qpsk_table(4) = -QPSK_LEVEL - QPSK_LEVEL*1i;

    % QSPK symbols containing a '0' and a '1' at the different bit positions
    S0_qpsk = zeros(2,2);
    S0_qpsk(1,:) = [1, 2];
    S0_qpsk(2,:) = [1, 3];
        
    S1_qpsk = zeros(2,2);
    S1_qpsk(1,:) = [3, 4];
    S1_qpsk(2,:) = [2, 4];
    
    
    %%
    % LTE-16QAM constellation:
	%                Q
	%  1011	   1001  |	 0001	0011
	%  1010	   1000  |	 0000	0010
	%---------------------------------> I
	%  1110    1100  |   0100	0110
	%  1111    1101  |   0101	0111
    
    if matlab_constellation
        D = 1;          % Testing: Matlab function outcomes assume 16QAM levels 1 and 3, rather than 1/sqrt(10) and 3/sqrt(10)
    else
        D = sqrt(10);  % 3GPP: TS 136 211 - V 10.5.0
    end
    QAM16_LEVEL_1 = 1/D;
    QAM16_LEVEL_2 = 3/D;

    % 16QAM modulation table
	qam16_table(1) = QAM16_LEVEL_1 + QAM16_LEVEL_1*1i;
	qam16_table(2) = QAM16_LEVEL_1 + QAM16_LEVEL_2*1i;
	qam16_table(3) = QAM16_LEVEL_2 + QAM16_LEVEL_1*1i;
	qam16_table(4) = QAM16_LEVEL_2 + QAM16_LEVEL_2*1i;
	qam16_table(5) = QAM16_LEVEL_1 - QAM16_LEVEL_1*1i;
	qam16_table(6) = QAM16_LEVEL_1 - QAM16_LEVEL_2*1i;
	qam16_table(7) = QAM16_LEVEL_2 - QAM16_LEVEL_1*1i;
	qam16_table(8) = QAM16_LEVEL_2 - QAM16_LEVEL_2*1i;
	qam16_table(9) = -QAM16_LEVEL_1 + QAM16_LEVEL_1*1i;
	qam16_table(10) = -QAM16_LEVEL_1 + QAM16_LEVEL_2*1i;
	qam16_table(11) = -QAM16_LEVEL_2 + QAM16_LEVEL_1*1i;
	qam16_table(12) = -QAM16_LEVEL_2 + QAM16_LEVEL_2*1i;
	qam16_table(13) = -QAM16_LEVEL_1 - QAM16_LEVEL_1*1i;
	qam16_table(14) = -QAM16_LEVEL_1 - QAM16_LEVEL_2*1i;
	qam16_table(15) = -QAM16_LEVEL_2 - QAM16_LEVEL_1*1i;
	qam16_table(16) = -QAM16_LEVEL_2 - QAM16_LEVEL_2*1i;
    
    % 16QAM symbols containing a '0' and a '1' at the different bit positions
    S0_qam16 = zeros(4,8);
    S0_qam16(1,:) = [1:8];      % symbols with a '0' at the bit position 0 (leftmost bit)
    S0_qam16(2,:) = [1:4, 9:12];   % symbols with a '0' at the bit position 1
    S0_qam16(3,:) = [1:2, 5:6, 9:10, 13:14];   % symbols with a '0' at the bit position 2
    S0_qam16(4,:) = [1, 3, 5, 7, 9, 11, 13, 15];   % symbols with a '0' at the bit position 3 (rightmost bit)
  
    S1_qam16 = zeros(4,8);
    S1_qam16(1,:) = [9:16];   % symbols with a '1' at the bit position 0 (leftmost bit)
    S1_qam16(2,:) = [5:8, 13:16];      % symbols with a '1' at the bit position 1
    S1_qam16(3,:) = [3:4, 7:8, 11:12, 15:16];      % symbols with a '1' at the bit position 2
    S1_qam16(4,:) = [2, 4, 6, 8, 10, 12, 14, 16];      % symbols with a '1' at the bit position 3 (rightmost bit)
    
    
    %%
    % LTE-64QAM constellation:
	% see [3GPP TS 36.211 version 10.5.0 Release 10, Section 7.1.4]
    
    %                                                     Q
    %  47-101111    45-101101   37-100101     39-100111   |     7-000111    5-000101    13-001101   15-001111
    %  46-101110    44-101100   36-100100     38-100110   |     6-000110    4-000100    12-001100   14-001110
	%  42-101010    40-101000   32-100000     34-100010   |     2-000010	0-000000    8-001000    10-001010
	%  43-101011    41-101001   33-100001     35-100011   |     3-000011	1-000001    9-001001    11-001011
	%-------------------------------------------------------------------------------------------------------------> I
    %  59-111011    57-111001   49-110001     51-110011   |     19-010011   17-010001   25-011001   27-011011
    %  58-111010    56-111000   48-110000     50-110010   |     18-010010   16-010000   24-011000   26-011010
	%  62-111110    60-111100   52-110100     54-110110   |     22-010110	20-010100   28-011100   30-011110
	%  63-111111    61-111101   53-110101     55-110111   |     23-010111	21-010101   29-011101   31-011111
	
    if matlab_constellation
        D = 1;          % Testing: Matlab function outcomes assume 64QAM levels 1, 3, 5 and 7, rather than 1/sqrt(42), 3/sqrt(42), 5/sqrt(42), and 7/sqrt(42),
    else
        D = sqrt(42);  % 3GPP: TS 136 211 - V 10.5.0
    end
    QAM64_LEVEL_1 = 1/D;
	QAM64_LEVEL_2 =	3/D;
	QAM64_LEVEL_3 =	5/D;
	QAM64_LEVEL_4 =	7/D;

    % 64QAM modulation table
    qam64_table(1) = QAM64_LEVEL_2 + QAM64_LEVEL_2*1i;
	qam64_table(2) = QAM64_LEVEL_2 + QAM64_LEVEL_1*1i;
	qam64_table(3) = QAM64_LEVEL_1 + QAM64_LEVEL_2*1i;
	qam64_table(4) = QAM64_LEVEL_1 + QAM64_LEVEL_1*1i;
	qam64_table(5) = QAM64_LEVEL_2 + QAM64_LEVEL_3*1i;
	qam64_table(6) = QAM64_LEVEL_2 + QAM64_LEVEL_4*1i;
	qam64_table(7) = QAM64_LEVEL_1 + QAM64_LEVEL_3*1i;
	qam64_table(8) = QAM64_LEVEL_1 + QAM64_LEVEL_4*1i;
	qam64_table(9) = QAM64_LEVEL_3 + QAM64_LEVEL_2*1i;
	qam64_table(10) = QAM64_LEVEL_3 + QAM64_LEVEL_1*1i;
	qam64_table(11) = QAM64_LEVEL_4 + QAM64_LEVEL_2*1i;
	qam64_table(12) = QAM64_LEVEL_4 + QAM64_LEVEL_1*1i;
	qam64_table(13) = QAM64_LEVEL_3 + QAM64_LEVEL_3*1i;
	qam64_table(14) = QAM64_LEVEL_3 + QAM64_LEVEL_4*1i;
	qam64_table(15) = QAM64_LEVEL_4 + QAM64_LEVEL_3*1i;
	qam64_table(16) = QAM64_LEVEL_4 + QAM64_LEVEL_4*1i;
	qam64_table(17) = QAM64_LEVEL_2 - QAM64_LEVEL_2*1i;
	qam64_table(18) = QAM64_LEVEL_2 - QAM64_LEVEL_1*1i;
	qam64_table(19) = QAM64_LEVEL_1 - QAM64_LEVEL_2*1i;
	qam64_table(20) = QAM64_LEVEL_1 - QAM64_LEVEL_1*1i;
	qam64_table(21) = QAM64_LEVEL_2 - QAM64_LEVEL_3*1i;
	qam64_table(22) = QAM64_LEVEL_2 - QAM64_LEVEL_4*1i;
	qam64_table(23) = QAM64_LEVEL_1 - QAM64_LEVEL_3*1i;
	qam64_table(24) = QAM64_LEVEL_1 - QAM64_LEVEL_4*1i;
	qam64_table(25) = QAM64_LEVEL_3 - QAM64_LEVEL_2*1i;
	qam64_table(26) = QAM64_LEVEL_3 - QAM64_LEVEL_1*1i;
	qam64_table(27) = QAM64_LEVEL_4 - QAM64_LEVEL_2*1i;
	qam64_table(28) = QAM64_LEVEL_4 - QAM64_LEVEL_1*1i;
	qam64_table(29) = QAM64_LEVEL_3 - QAM64_LEVEL_3*1i;
	qam64_table(30) = QAM64_LEVEL_3 - QAM64_LEVEL_4*1i;
	qam64_table(31) = QAM64_LEVEL_4 - QAM64_LEVEL_3*1i;
	qam64_table(32) = QAM64_LEVEL_4 - QAM64_LEVEL_4*1i;
	qam64_table(33) = -QAM64_LEVEL_2 + QAM64_LEVEL_2*1i;
	qam64_table(34) = -QAM64_LEVEL_2 + QAM64_LEVEL_1*1i;
	qam64_table(35) = -QAM64_LEVEL_1 + QAM64_LEVEL_2*1i;
	qam64_table(36) = -QAM64_LEVEL_1 + QAM64_LEVEL_1*1i;
	qam64_table(37) = -QAM64_LEVEL_2 + QAM64_LEVEL_3*1i;
	qam64_table(38) = -QAM64_LEVEL_2 + QAM64_LEVEL_4*1i;
	qam64_table(39) = -QAM64_LEVEL_1 + QAM64_LEVEL_3*1i;
	qam64_table(40) = -QAM64_LEVEL_1 + QAM64_LEVEL_4*1i;
	qam64_table(41) = -QAM64_LEVEL_3 + QAM64_LEVEL_2*1i;
	qam64_table(42) = -QAM64_LEVEL_3 + QAM64_LEVEL_1*1i;
	qam64_table(43) = -QAM64_LEVEL_4 + QAM64_LEVEL_2*1i;
	qam64_table(44) = -QAM64_LEVEL_4 + QAM64_LEVEL_1*1i;
	qam64_table(45) = -QAM64_LEVEL_3 + QAM64_LEVEL_3*1i;
	qam64_table(46) = -QAM64_LEVEL_3 + QAM64_LEVEL_4*1i;
	qam64_table(47) = -QAM64_LEVEL_4 + QAM64_LEVEL_3*1i;
	qam64_table(48) = -QAM64_LEVEL_4 + QAM64_LEVEL_4*1i;
	qam64_table(49) = -QAM64_LEVEL_2 - QAM64_LEVEL_2*1i;
	qam64_table(50) = -QAM64_LEVEL_2 - QAM64_LEVEL_1*1i;
	qam64_table(51) = -QAM64_LEVEL_1 - QAM64_LEVEL_2*1i;
	qam64_table(52) = -QAM64_LEVEL_1 - QAM64_LEVEL_1*1i;
	qam64_table(53) = -QAM64_LEVEL_2 - QAM64_LEVEL_3*1i;
	qam64_table(54) = -QAM64_LEVEL_2 - QAM64_LEVEL_4*1i;
	qam64_table(55) = -QAM64_LEVEL_1 - QAM64_LEVEL_3*1i;
	qam64_table(56) = -QAM64_LEVEL_1 - QAM64_LEVEL_4*1i;
	qam64_table(57) = -QAM64_LEVEL_3 - QAM64_LEVEL_2*1i;
	qam64_table(58) = -QAM64_LEVEL_3 - QAM64_LEVEL_1*1i;
	qam64_table(59) = -QAM64_LEVEL_4 - QAM64_LEVEL_2*1i;
	qam64_table(60) = -QAM64_LEVEL_4 - QAM64_LEVEL_1*1i;
	qam64_table(61) = -QAM64_LEVEL_3 - QAM64_LEVEL_3*1i;
	qam64_table(62) = -QAM64_LEVEL_3 - QAM64_LEVEL_4*1i;
	qam64_table(63) = -QAM64_LEVEL_4 - QAM64_LEVEL_3*1i;
	qam64_table(64) = -QAM64_LEVEL_4 - QAM64_LEVEL_4*1i;
    
    % 64QAM symbols containing a '0' and a '1' at the different bit positions
    S0_qam64 = zeros(6,32);
    S0_qam64(1,:) = [1:32];      % symbols with a '0' at the bit position 0 (leftmost bit)
    S0_qam64(2,:) = [1:16, 33:48];   % symbols with a '0' at the bit position 1
    S0_qam64(3,:) = [1:8, 17:24, 33:40, 49:56];   % symbols with a '0' at the bit position 2
    S0_qam64(4,:) = [1:4, 9:12, 17:20, 25:28, 33:36, 41:44, 49:52, 57:60];   % symbols with a '0' at the bit position 3
    S0_qam64(5,:) = [1:2, 5:6, 9:10, 13:14, 17:18, 21:22, 25:26, 29:30, 33:34, 37:38, 41:42, 45:46, 49:50, 53:54, 57:58, 61:62];   % symbols with a '0' at the bit position 4
    S0_qam64(6,:) = [1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63];   % symbols with a '0' at the bit position 5 (rightmost bit)
  
    S1_qam64 = zeros(6,32);
    S1_qam64(1,:) = [33:64];   % symbols with a '1' at the bit position 0 (leftmost bit)
    S1_qam64(2,:) = [17:32, 49:64];      % symbols with a '1' at the bit position 1
    S1_qam64(3,:) = [9:16, 25:32, 41:48, 57:64];      % symbols with a '1' at the bit position 2
    S1_qam64(4,:) = [5:8, 13:16, 21:24, 29:32, 37:40, 45:48, 53:56, 61:64];      % symbols with a '1' at the bit position 3
    S1_qam64(5,:) = [3:4, 7:8, 11:12, 15:16, 19:20, 23:24, 27:28, 31:32, 35:36, 39:40, 43:44, 47:48, 51:52, 55:56, 59:60, 63:64];      % symbols with a '1' at the bit position 4
    S1_qam64(6,:) = [2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64];      % symbols with a '1' at the bit position 5 (rightmost bit)


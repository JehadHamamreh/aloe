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

function [sc, zero_points, cp_samples, cp_samples_first, bits_per_symbol, fs] = configure_params(fft_points, normal, modulation)

    % FFT points/:
    fft128 = 128;   sc_fft128 = 72;     zeros_fft128 = fft128 - sc_fft128;      % Max. Tr-Bw = 6*12*15 kHz = 1.08 MHz (Ch-Bw = 1.4 MHz)
    fft256 = 256;   sc_fft256 = 180;    zeros_fft256 = fft256 - sc_fft256;      % Max. Tr-Bw = 15*12*15 kHz = 2.7 MHz (Ch-Bw = 3 MHz)
    fft512 = 512;   sc_fft512 = 300;    zeros_fft512 = fft512 - sc_fft512;      % Max. Tr-Bw = 25*12*15 kHz = 4.5 MHz (Ch-Bw = 5 MHz)
    fft1024 = 1024; sc_fft1024 = 600;   zeros_fft1024 = fft1024 - sc_fft1024;   % Max. Tr-Bw = 50*12*15 kHz = 9 MHz (Ch-Bw = 10 MHz)
    fft1536 = 1536; sc_fft1536 = 900;   zeros_fft1536 = fft1536 - sc_fft1536;   % Max. Tr-Bw = 75*12*15 kHz = 13.5 MHz (Ch-Bw = 15 MHz)
    fft2048 = 2048; sc_fft2048 = 1200;  zeros_fft2048 = fft2048 - sc_fft2048;   % Max. Tr-Bw = 100*12*15 kHz = 18 MHz (Ch-Bw = 20 MHz)

    % Cyclic prefix samples:
    samples_fft128 = 9;     samples_fft128_first = 10;  samples_fft128e =32;
    samples_fft256 = 18;    samples_fft256_first = 20;  samples_fft256e = 64;
    samples_fft512 = 36;    samples_fft512_first = 40;  samples_fft512e = 128;
    samples_fft1024 = 72;   samples_fft1024_first = 80; samples_fft1024e = 256;
    samples_fft1536 = 108;  samples_fft1536_first = 120;samples_fft1536e = 384;
    samples_fft2048 = 144;  samples_fft2048_first = 160;samples_fft2048e = 512;

    switch modulation
        case 0, bits_per_symbol=1;  % BPSK
        case 1, bits_per_symbol=2;  % QPSK
        case 2, bits_per_symbol=4;  % 16QAM
        case 3, bits_per_symbol=6;  % 64QAM
    end
        
    switch fft_points
        case fft128
            sc = sc_fft128; zero_points = zeros_fft128; 
            if (normal)
                cp_samples = samples_fft128; cp_samples_first = samples_fft128_first;
            else
                cp_samples = samples_fft128e; cp_samples_first = samples_fft128e;
            end
            fs = 1.92e6;
        case fft256
            sc = sc_fft256; zero_points = zeros_fft256;
            if (normal)
                cp_samples = samples_fft256; cp_samples_first = samples_fft256_first;
            else
                cp_samples = samples_fft256e; cp_samples_first = samples_fft256e;
            end
            fs = 3.84e6;
        case fft512
            sc = sc_fft512; zero_points = zeros_fft512;
            if (normal)
                cp_samples = samples_fft512; cp_samples_first = samples_fft512_first;
            else
                cp_samples = samples_fft512e; cp_samples_first = samples_fft512e;
            end
            fs = 7.68e6;
        case fft1024
            sc = sc_fft1024; zero_points = zeros_fft1024;
            if (normal)
                cp_samples = samples_fft1024; cp_samples_first = samples_fft1024_first;
            else
                cp_samples = samples_fft1024e; cp_samples_first = samples_fft1024e;
            end
            fs = 15.36e6; % 15.36 MHz (df = 15.36/1024 = 15 kHz sub-carrier spacing)
        case fft1536
            sc = sc_fft1536; zero_points = zeros_fft1536;
            if (normal)
                cp_samples = samples_fft1536; cp_samples_first = samples_fft1536_first;
            else
                cp_samples = samples_fft1536e; cp_samples_first = samples_fft1536e;
            end
            fs = 23.04e6; % 3.84 MHz (df = 23.04/1536 = 15 kHz sub-carrier spacing)
        case fft2048
            sc = sc_fft2048; zero_points = zeros_fft2048;
            if (normal)
                cp_samples = samples_fft2048; cp_samples_first = samples_fft2048_first;
            else
                cp_samples = samples_fft2048e; cp_samples_first = samples_fft2048e;
            end
            fs = 30.72e6; % 30.72 MHz (df = 30.72/2048 = 15 kHz sub-carrier spacing)
    end
end
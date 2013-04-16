% Test OFDM modem


%% Paths to m and MEX files
% m-files:
addpath('../common functions');

% MEX-files:
addpath('/usr/local/mex');

%% Parameters
ofdm_symbols_per_subframe = 2;
bits_per_symbol = 4;
modulator_params={{'modulation',int32(2)}};
fft_points = 512;
sc = 600;
UL = 1;
count = 0;  % sample count

    % Generate random input bis
    in_bits = randi(2,1,(ofdm_symbols_per_subframe*sc)*bits_per_symbol)==1;     % should be integer divisible by 'ofdm_symbols_per_slot'
    length(in_bits)

    %% Symbol level
    % ********** TRANSMITTER **********
    % Modulation
    out_modulator = am_gen_modulator(in_bits, modulator_params);
    
    % DFT in case of UL
    if (UL)
        % 12 or 14 OFDM Symbols per subframe
        for k=0:(ofdm_symbols_per_subframe-1)
        % Transform Precoding, Tx (UL only), should execute only 12 times1
        % per subframe (1 ms). The remaining 2 OFDM symbols are fed with control data?
            out_dft(k*sc+1:(k+1)*sc) = 1/sqrt(sc)*fft(out_modulator(k*sc+1:(k+1)*sc));
        end
    else
        out_dft = out_modulator;
    end
    
        % OFDM/SC-FDMA Modulator: Includes padding of zeros (unused subcarriers)
        if (UL)
            % SC-FDMA modem is like OFDM modem, except that DC is not set to 0
            out_ifft = sc_fdma_modem(out_dft, ofdm_symbols_per_subframe, fft_points, sc, 1, count);
        else
            out_ifft = ofdm_modem(out_dft,ofdm_symbols_per_subframe, fft_points, sc, 1);
        end
      
        % OFDM/SC-FDMA Demodulator: Includes unpadding of zeros (unused subcarriers)
       	if (UL)
        	% SC-FDMA modem is like OFDM modem, except that DC is not set
            % to 0 and spectrum centered between subcarrier index sc/2-1 and
            % sc/2
            out_fft = sc_fdma_modem(out_ifft, ofdm_symbols_per_subframe, fft_points, sc, 0, count);
        else
        	out_fft = ofdm_modem(out_ifft, ofdm_symbols_per_subframe, fft_points, sc, 0);
        end

        
    % IDFT in case of UL
	if (UL)
        for k=0:(ofdm_symbols_per_subframe-1) 
            % Transform Precoding, Rx (UL only)
            out_idft(k*sc+1:(k+1)*sc) = sc/sqrt(sc)*ifft(out_fft(k*sc+1:(k+1)*sc));
        end
    else
    	out_idft = out_fft;
    end
    
    
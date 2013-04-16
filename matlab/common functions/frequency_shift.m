% Frequency shift function
% Computes samples of complex phasor

% N             - fft/ifft points
%
% f0            - center frequency of complex phasor
%
% count         - initial sample
%

function shift = frequency_shift(N, ofdm_symbols, f0, count, direction)
    
    % obtain sampling rate for frequency shift
    switch (N)
        case 128,
            fs = 1.92e6; % 1.92 MHz (df = 1.92/128 = 15 kHz sub-carrier spacing)
        case 256,
            fs = 3.84e6; % 3.84 MHz (df = 3.84/256 = 15 kHz sub-carrier spacing)
        case 512,
            fs = 7.68e6; % 7.68 MHz (df = 7.68/512 = 15 kHz sub-carrier spacing)    
        case 1024,
            fs = 15.36e6; % 15.36 MHz (df = 15.36/1024 = 15 kHz sub-carrier spacing)
        case 1536,
            fs = 23.04e6; % 3.84 MHz (df = 23.04/1536 = 15 kHz sub-carrier spacing)
        case 2048,
            fs = 30.72e6; % 30.72 MHz (df = 30.72/2048 = 15 kHz sub-carrier spacing)
        otherwise
            fprintf('\nIncorrect number of fft/ifft points (%d). See LTE specification.\n', points);
            return;
    end
    
    % Compute samples of complex phasor for frequency shift
    t=count:(count+N*ofdm_symbols-1);
    if (direction)
        shift = exp(2i*pi*f0*t/fs);
        %shift = cos(2*pi*f0*t/fs) + 1i*sin(2*pi*f0*t/fs);
    else
    	shift = exp(-2i*pi*f0*t/fs);
        %shift = cos(2*pi*f0*t/fs) - 1i*sin(2*pi*f0*t/fs);
    end
    
end

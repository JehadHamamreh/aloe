
function out = unpadding(in, fft_points, sc)

    l_in = length(in);
    nof_dfts = l_in/fft_points;
    if (nof_dfts > floor(l_in/fft_points)) % not integer divisible
       fprintf('\nNumber of input samples %d must be multiple of DFT size %d', l_in, fft_points);
       return;
    end
    z = fft_points - sc;
    z_half = z/2;
    if (z_half > floor(z/2))
       fprintf('\nNumber of zeros %d to be unpadded not even.', z);
       return;
    end
    
    for i=0:(nof_dfts-1)
        out(i*sc+1:(i+1)*sc) = in(i*fft_points+z_half+1:i*fft_points+z_half+sc);
    end
    
end

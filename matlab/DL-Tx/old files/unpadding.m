
function out = unpadding(in, fft_points, slots, z)

    for i=1:slots
        out((i-1)*fft_points+1:i*fft_points) = in((i-1)*(fft_points+(2*i-1)*z)+1:i*(fft_points+(2*i-1)*z));
    end
    
end

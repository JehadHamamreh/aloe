function out = cyclic_detach(in, ofdm_symbols_per_subframe, fft_points, cp_samples, cp_samples_first)

    l_in = length(in);

    out = zeros(1,fft_points);

%     slots = (in/((fft_points+cp_samples_first)+((ofdm_symbols_per_slot-1)*(fft_points+cp_samples_first)));
%     if (mod(slots,2*ofdm_symbols_per_slot) ~=0)
%         fprintf('\nWarning: Last slot partially empty.');
%     end
%     fprintf('\nNumber of slots: ');
%     slots
    
    %for i=1:slots
    k=0;
    discarded = 0;
    for k=0:(ofdm_symbols_per_subframe-1)
        if (k==0)
            discarded = discarded + cp_samples_first;
            out(k*fft_points+1:(k+1)*fft_points) = in(k*fft_points+discarded+1:(k+1)*fft_points+discarded);
        else
            discarded = discarded + cp_samples;
            out(k*fft_points+1:(k+1)*fft_points) = in(k*fft_points+discarded+1:(k+1)*fft_points+discarded);
        end
        k=k+1;
    end

end

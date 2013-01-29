function [ output ] = lte_channel( input, params)

    noise = sqrt(params.fft_size/params.Ntot)*(sqrt(params.sigma2)*(randn(size(input))+1i*randn(size(input))));
    output=input+noise;

end


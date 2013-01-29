function [ output ] = lte_channel( input, params)

    output=input+sqrt(params.sigma2)*randn(size(input));

end


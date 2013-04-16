% Code Block Concatenation: Sequentially concatenates the samples contained
% in different code blocks
%

% Inputs:      
% in0           - 2D Matrix of CxM elements, containing the coded and rate
%               matched bits
%               Each row represent a code block (output of one coder+
%               rate_matching instance)
%               
% Outputs:     
% out           - Vector containing the coded, rate matched and 
%               concatenated bits
%               Ml: symbols per layer (equal for all layers)
%
% Spec:         3GPP TS 36.212 section 5.1.5 v10.6.0
% Notes:        -
% 

function out = gen_code_block_concatenation(in)

    [C, M] = size(in);
    
    out = zeros(1,C*M);
    % concatenation row-wise: row by row of 'in' is joined in the row vector
    % 'out' (substitute the following by double-loop in C)
    for i=1:C
        out((i-1)*M+1:i*M) = in(i,:);
    end
    
    
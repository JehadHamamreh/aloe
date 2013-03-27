

% test repetition coding & decoding

D = 31;
input = (randi(2,1,D)==1);
rep = 3;    % coding rate: 1/rep

out_coder = repetition_coding(input, rep)
out_decoder = repetition_decoding(out_coder, rep)


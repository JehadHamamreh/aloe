clear;
N=100;
addpath('/usr/local/mex')

trellis=poly2trellis(7,[133 177 165]);
mex_opts = {{'constraint_length',int32(7)},{'tail_bit',int32(1)},{'rate',int32(3)}, ...
    {'generator_0',int32(91)},{'generator_1',int32(127)},{'generator_2',int32(117)}, ...
    {'padding',int32(0)}};

input=randi(2,N,1)==1;

mex_cc=am_gen_convcoder(input,mex_opts);
cc=convenc(input,trellis);

%if (sum(mex_cc~=cc')) 
%    fprintf('Generated sequence does not match Matlab convenc\n');
%    break;
%end

llr = zeros(size(cc));
llr(cc==1)=-10.0;
llr(cc==0)=10.0;

mex_out=am_gen_viterbi(llr,mex_opts);

fprintf('total errors: %d\n',sum(mex_out(1:N)~=input'));
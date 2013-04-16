addpath('/usr/local/mex')
addpath('../common functions')
N=115;
modulation=3;
sigma2=1;
hard=1; % if 1, soft value is ignored
soft=0;
zero=0;
one=1;
debug=0;

% generates random complex symbols, equally distributed between -sqrt(2)
% and +sqrt(2)
x=sqrt(2)*[rand(N,1)-0.5*ones(N,1)] + 1i*sqrt(2)*[rand(N,1)-0.5*ones(N,1)];

if (hard == 1)
    out_mex=am_gen_hard_demod(x, {{'modulation',int32(modulation)}});
    out_matlab=soft_demapper(x, modulation, 0, zero, one, sigma2, debug);
else
    out_mex=am_gen_soft_demod(x, {{'soft',int32(soft)},{'modulation',int32(modulation)},{'sigma2',sigma2}}); % soft: 0 means exact LLR and 1 approx. LLR
    out_matlab=soft_demapper(x, modulation, soft+1, zero, one, sigma2, debug); % third parameter: 0 means hard, 1 exact LLR, and 2 approx. LLR
end

switch (modulation)
    case 0 % BPSK
        bits_per_symbol = 1;
    case 1 % QPSK
        bits_per_symbol = 2;
    case 2 % QAM16
        bits_per_symbol = 4;
    case 3 % QAM64
        bits_per_symbol = 6;
end
    
plot(1:bits_per_symbol*N,out_matlab,'b-',1:bits_per_symbol*N,out_mex,'r--');
axis tight;

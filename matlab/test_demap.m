addpath('/usr/local/mex')
%addpath('/home/vuk/DATOS/workspace/gen_soft_demod')
N=541;
modulation=1;
sigma2=1.5;

% generates random complex symbols, equally distributed between -sqrt(2)
% and +sqrt(2)
sig=sqrt(2)*[rand(N,1)-0.5*ones(N,1)] + 1i*sqrt(2)*[rand(N,1)-0.5*ones(N,1)];
u=load('unlayered.mat');
u=u.unlayered_pdcch;
x=sig;
x=u';
out_mex=am_gen_soft_demod(x, {{'soft',int32(1)},{'modulation',int32(2)},{'sigma2',1.5}}); % soft = 0: exact; soft = 1: approx. LLR
out_matlab=soft_demapper(x, 1, 2, 1.5); % 2 means approx. LLR, 1 exact

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

N=10;
plot(1:bits_per_symbol*N,out_matlab(1:bits_per_symbol*N),'b-',1:bits_per_symbol*N,out_mex(1:bits_per_symbol*N),'r--');
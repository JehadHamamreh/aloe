addpath('/usr/local/mex')
addpath('../common functions')
N=240;
QPSK=1;
modulation=QPSK;

x=randi(2,N,1)==1;

out_mex=am_gen_modulator(x, {{'modulation',int32(modulation)}})
out_matlab=qpsk_modulation(x)

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
    
subplot(2,1,1),plot(1:floor(N/bits_per_symbol),real(out_matlab),'b-',1:floor(N/bits_per_symbol),real(out_mex),'r--');
axis tight;
subplot(2,1,2),plot(1:floor(N/bits_per_symbol),imag(out_matlab),'b-',1:floor(N/bits_per_symbol),imag(out_mex),'r--');
axis tight;

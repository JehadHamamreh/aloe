%%%%%% Modulation Demapping, hard and soft TESTBENCH %%%%%%
% tests the custom soft demapping algorithms against the available Matlab
% functions

addpath('../common functions/modulation');

% Configuration
N = 100;            % number of complex symbols to demodulate

% indicates the hard decision values for a '0' and '1', which could be 0 and 1 or -100 and 100, for instance
zero = 0;
one = 1;

change_sign = -1;   % factor changing the sign. The LLR is positive when you are "closer" to a '0' then a '1'. The decoder assumes the opposite.

matlab_constellation = 1;  % set to 1 when testing against Matlab demodulators
% This parameter can be omitted for C implementation

% modulation parameters
BPSK = 0;
QPSK = 1;
QAM16 = 2;
QAM64 = 3;

modulation = QPSK;

switch modulation
    case BPSK,  M = 2; % M: number of constellation points
    case QPSK,  M = 4;
    case QAM16, M = 16;
    case QAM64, M = 64;
end;

[bpsk_table, S0_bpsk, S1_bpsk, qpsk_table, S0_qpsk, S1_qpsk, qam16_table, S0_qam16, S1_qam16, qam64_table, S0_qam64, S1_qam64] = constellation_tables(matlab_constellation);

% demodulation parameters
sigma2 = 1;  % noise variance

% set one of the following
soft = 1;   % 0: hard decision; 1: LLR; 2: approximate LLR

% generate random symbols within the margins of the outmost symbols of the
% given constellation diagram
in = zeros(N,1);
in = generate_symbols(modulation, N, matlab_constellation);


%%% DEMODULATION %%%
if (soft == 0)  % HARD DEMODULATION: demodulation is specific for the given symbol constellation
    switch modulation
        case BPSK
            out = hard_bpsk(in, zero, one);
        case QPSK
            out = hard_qpsk(in, zero, one);
        case QAM16
            out = hard_qam16(in, zero, one, matlab_constellation);
        case QAM64
            out = hard_qam64(in, zero, one, matlab_constellation);
    end
    
else % SOFT DEMODULATION - LLR: (initial) algorithm (implementation -- non-optimized) is regular, independent of symbol constellation
    switch modulation
        case BPSK
            constellation = bpsk_table; S0 = S0_bpsk; S1 = S1_bpsk;
        case QPSK
            constellation = qpsk_table; S0 = S0_qpsk; S1 = S1_qpsk;
        case QAM16
            constellation = qam16_table; S0 = S0_qam16; S1 = S1_qam16;
        case QAM64
            constellation = qam64_table; S0 = S0_qam64; S1 = S1_qam64;
    end
    
    %if (soft == 1)  % exact LLR
        out1 = change_sign*llrexact(in, constellation, S0, S1, sigma2);
    %else            % approximate LLR
        out2 = change_sign*llrapprox(in, constellation, S0, S1, sigma2);
    %end
end

% Reference demodulation using Matlab functions
out_ref1 = ref_demod(in, M, 1, sigma2);
out_ref2 = ref_demod(in, M, 2, sigma2);

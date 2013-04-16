% LTE Downlink Transmitter Processing Chain
%
% Mix of Matlab m functions and MEX files
%

%% Paths to m and MEX files
% m-files:
addpath('../common functions');

% MEX-files:
addpath('/usr/local/mex');

%% Parameters
BPSK = 0; QPSK = 1; QAM16 = 2; QAM32 = 3;   % modulation types
single_ap = 0; tx_div = 1; sp_mux = 2;      % MIMO concepts
Ncycles = 20;
% Transmission parameters defining the WF transmission mode:
parity_bits = 24;
ofdm_symbols_per_slot = 7;
fft_points = 128;
sub_carriers = 72;
unused_subcarriers = fft_points - sub_carriers;
if (mod(sub_carriers,2) ~= 0)
    fprintf('\nWarning: Unused subcarriers not integer divisible by 2. Be cautious with the resource mapping.');
end

mode = 2;
% MIMO modes:
if (mode == 0)
    q = 1; v = 1; p = 0; style = single_ap;
elseif (mode == 1)
    q = 1;  % Number of transport blocks (code blocks)
    v = 2;  % Number of layers (see valid combination @lte_PDSCH_layer.m)
    p = [0,1];  % antenna port vector (see lte_PDSCH_precoding.m)
    style = tx_div; % MIMO mode
elseif (mode == 2)
    q = 1;  % Number of transport blocks (code blocks)
    v = 4;  % Number of layers (see valid combination @lte_PDSCH_layer.m)
    p = [0,1,2,3];  % antenna port vector (see lte_PDSCH_precoding.m)
    style = tx_div; % MIMO mode    
else
    q = 1;  % Number of transport blocks (code blocks)
    v = 4;  % Number of layers (see valid combination @lte_PDSCH_layer.m)
    p = [0,1,2,3];  % antenna port vector (see lte_PDSCH_precoding.m)
    style = sp_mux; % MIMO mode    
end

ap = length(p);

% Function parameters:
% crc_params:          'long_crc' = parity_bits
%
% seg_params:           ...?
%
% coder_params:        'padding' = 0, ..., ??? (padded zeros?)
%
% modulator_params:    'modulation' = BPSK, QPSK, QAM16, QAM32
%
% padding_params:      'data_type' = int32(0); 
%                       'pre_padding' = int32(prep) (prep: number of zeros padded at beginning);
%                       'post_padding' = int32(postp) (postp: number of zeros
%                       padded at end);

crc_params={{'long_crc',int32(parity_bits)}};
coder_params={{'padding',int32(0)}};
modulator_params={{'modulation',int32(QPSK)}};
padding_params={{'data_type',int32(0)},{'pre_padding',int32(unused_subcarriers/2)},{'post_padding',int32(unused_subcarriers/2)},{'nof_packets',int32(ofdm_symbols_per_slot)}};
ifft_params={{'direction',int32(1)},{'mirror',int32(1)},{'normalize',int32(1)},{'dft_size',int32(fft_points)}};
cyclic_params={{'ofdm_symbol_sz',int32(128)},{'cyclic_prefix_sz',int32(9)},{'first_cyclic_prefix_sz',int32(10)}};

output=[];
for i=1:Ncycles
    in_bits = randi(2,308,1)==1;
	
%	out_crc = am_gen_crc(in_bits,crc_params);	

    %% Bit level: Code block processing
    
%     % El segementation + segundo CRC debería devolver una matriz donde cada
%     % fila, p.j. representa un codeblock con el numero de muestras que hay
%     % entonces el numero de filas determina el numero de instancias del
%     % coder y rate matching y el numero de entradas del Code Blk Concat.
%     out_seg = am_gen_seg(out_crc,seg_params); % asumiendo que el 2º CRC se hace implicitamente
%     
%     [C, samples_per_CB] = size(out_seg);    % C code blocks (# data otputs of segmentation/ 2nd crc of 'samples_per_CB' samples each)
%     
%     out_encoder = zeros(C, 3*samples_per_CB);    % El coding rate es 1/3 (siempre?), lo que quiere decir que por cada bit de entrada al coder salen 3 bits (fijo?)
%     out_rm = [];    % El numero de muestras a la salida del RM es conocido a priori?
%     j=1:C;
%         out_encoder(j,:) = am_lte_encoder(out_seg(j,:),coder_params);
%         out_rm(j,:) = am_lte_rm(out_encoder(j,:),rm_params);
%     
%     out_concat = am_gen_concat(out_rm,concat_params);

%    out_encoder = am_lte_encoder(out_crc,coder_params);
    
    %out_concat = concat(out_encoder);   % No parameters, conctenates the enconder outputs (in case of multiple encoders)
    
    %% Symbol level
 
    out_modulator = am_gen_modulator(in_bits,modulator_params);	
    
    out_layer = lte_PDSCH_layer_mapper(out_modulator, 0, v, q, style)

    out_precoding = lte_PDSCH_precoding(out_layer, p, style)
    
    for l=1:ap
        out_padding = am_gen_padding(out_precoding(l,:),padding_params);

        out_ifft = am_gen_dft(out_padding,ifft_params);	

        out_cyclic = am_gen_cyclic(out_ifft,cyclic_params);	

        output(l,:) = [out_cyclic];
    end
end

for l=1:ap
    figure;
    plot(10*log10(pwelch(output(l,:))));
end

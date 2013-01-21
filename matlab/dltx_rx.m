Ncycles = 20;
modulation = 1;
mode = 0;

fft_points = 128;    
sc = 72;    
zero_points = 128 - 72;
ofdm_symbols_per_slot = 7;    

nofSYMBxSlot=[132 204 276 480 276 480 276 480 276 480 132 480 276 480 276 480 276 480 276 480]; 

modulator_params={{'modulation',int32(modulation)}};
padding_params={{'data_type',int32(0)},{'pre_padding',int32(zero_points/2)},{'post_padding',int32(zero_points/2)},{'nof_packets',int32(ofdm_symbols_per_slot)}};
ifft_params={{'direction',int32(1)},{'mirror',int32(0)},{'normalize',int32(1)},{'dft_size',int32(fft_points)}};
cyclic_params={{'ofdm_symbol_sz',int32(128)},{'cyclic_prefix_sz',int32(9)},{'first_cyclic_prefix_sz',int32(10)}};


%% DSP
output=[]; 
input=[];  
out_freq=[];
for i=1:Ncycles
    in_bits = randi(2,2*nofSYMBxSlot(i),1)==1;
	    
    out_modulator = am_gen_modulator(in_bits,modulator_params);	
    out_padding = am_lte_resourMappD(out_modulator,{{'slot_idx',int32(i-1)}});
    out_freq = [out_freq out_padding];
    out_ifft = am_gen_dft(out_padding,ifft_params);
    out_cyclic = am_gen_cyclic(out_ifft,cyclic_params);
    output = [output out_cyclic]; 
    
end

find_pss_freq(out_freq);

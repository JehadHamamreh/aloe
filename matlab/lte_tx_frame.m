function [ output ] = lte_tx_frame( input, params )
    
    crc_params={{'long_crc',int32(24)}};
    coder_params={{'direction',int32(0)},{'padding',int32(0)}};
    modulator_params={{'direction',int32(0)},{'modulation',int32(params.modulation)}};
    ifft_params={{'direction',int32(1)},{'mirror',int32(1)},{'normalize',int32(1)},{'dft_size',int32(params.fft_size)}};

    output=zeros(params.nof_slots,params.nof_samples_x_slot);
    k=1;
    for s=1:params.nof_slots
        rm_params={{'direction',int32(0)},{'tslot_idx',int32(s-1)},{'rvidx',int32(0)},{'mcs',int32(params.mcs)},{'nrb',int32(params.nrb)},{'fft_size',int32(params.fft_size)},{'cp_is_long',int32(0)},{'lteslots_x_timeslot',int32(params.lteslots_x_timeslot)}};
        resmap_params={{'direction',int32(0)},{'tslot_idx',int32(s-1)},{'lteslots_x_timeslot',int32(params.lteslots_x_timeslot)}};

        tb_crc=am_gen_crc(input(k:(k+params.src_bits_x_slot(s)-1)),crc_params); 
        k=k+params.src_bits_x_slot(s);

        cb_enc=am_lte_turbocode(tb_crc,coder_params);

        cw_rm=am_lte_ratematching(cb_enc,rm_params);        
        cw_mod = am_gen_modem(cw_rm,modulator_params);
        cw_resmap = am_lte_resource_mapper(cw_mod,resmap_params);
        sym = am_gen_demux(cw_resmap,{{'nof_outputs',int32(params.nof_fft_x_slot)},{'data_type',int32(2)}});
        cw_cp=cell(params.nof_fft_x_slot,1);
        for i=1:params.nof_fft_x_slot
            cyclic_params={{'ofdm_symbol_sz',int32(params.fft_size)},{'cyclic_prefix_sz',int32(params.cp(i))}};
            cw_fft = am_gen_dft(sym{i},ifft_params);
            cw_cp{i} = am_gen_cyclic(cw_fft,cyclic_params);
%            output(s,j:(j+params.fft_size+params.cp(i)-1))=cw_cp;      
%            j = j+length(cw_cp);
        end
        output(s,:)=am_gen_mux(cw_cp,{{'nof_inputs',int32(params.nof_fft_x_slot)},{'data_type',int32(2)}});
    end
    
end


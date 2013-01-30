function [ output ] = lte_rx_frame( input, params )

    fft_params={{'direction',int32(0)},{'mirror',int32(2)},{'normalize',int32(1)},{'dft_size',int32(params.fft_size)}};

    output=[];
    for s=1:params.nof_slots
        unrm_params={{'direction',int32(1)},{'tslot_idx',int32(s-1)},{'rvidx',int32(0)},{'mcs',int32(params.mcs)},{'nrb',int32(params.nrb)},{'fft_size',int32(params.fft_size)},{'cp_is_long',int32(0)},{'lteslots_x_timeslot',int32(params.lteslots_x_timeslot)}};

        % time synchronization
        sym = am_gen_demux(input(s,:),{{'nof_outputs',int32(params.nof_fft_x_slot)},{'out_len_0',int32(params.fft_size+params.cp(1))},{'out_len_7',int32(params.fft_size+params.cp(8))},{'data_type',int32(2)}});

        rx_fft=cell(params.nof_fft_x_slot,1);
        for i=1:params.nof_fft_x_slot
            remcyclic_params={{'ofdm_symbol_sz',int32(params.fft_size)},{'cyclic_prefix_sz',int32(params.cp(i))}};
            rx_cp = am_gen_remcyclic(sym{i},remcyclic_params);
            rx_fft{i} = am_gen_dft(rx_cp,fft_params);
        end
        
        slot_t=am_gen_mux(rx_fft,{{'nof_inputs',int32(params.nof_fft_x_slot)},{'data_type',int32(2)}});
        
        [phy_chan]=am_lte_resource_mapper(slot_t,{{'direction',int32(1)},{'tslot_idx',int32(s-1)},{'lteslots_x_timeslot',int32(params.lteslots_x_timeslot)}});
        pdsch=phy_chan{1};
        
        %rx_pdsch = am_lte_equalizer({pdsch,refsig},{});
        
        cb_mod = am_gen_soft_demod(pdsch, {{'soft',int32(1)},{'modulation',int32(params.modulation)},{'sigma2',params.sigma2}}); % soft = 0: exact; soft = 1: approx. LLR
        cb_rm=am_lte_ratematching(cb_mod,unrm_params);
        cb_dec=am_lte_turbocode(cb_rm,{{'direction',int32(1)}});
        tb_crc=am_gen_crc(cb_dec,{{'direction',int32(1)}});
        output=[output tb_crc];
    end

end


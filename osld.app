main:
{
	/* if selected, each module is called waveform_granularity_us/time_slot times per timeslot */
	/* if set to non-zero, the platform time slot must be integer divisible of waveform_granularity_us */
	waveform_granularity_us=0;
	
	precach_pipeline=false;
	auto_ctrl_module="ctrl";
};

modules:
{
	/* muxes all control messages from other modules */
	ctrl_mux: 
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=15;
		variables=(
			{name="nof_inputs";value=4;},{name="data_type";value=0;});
	};
	
	ctrl:
	{
		binary="modrep_osld/liblte_ctrl.so";	
		mopts=100;
		variables=(
			{name="nof_output_data_itf";value=0},
			{name="mode";value=2}, /* 0 tx, 1 rx, 2 both */
			{name="mcs_tx";value=9},{name="nof_rbg_tx";value=6},{name="rbg_mask_tx";value=0x3f},
			{name="nof_prb_tx";value=6},
			
			{name="cfi_tx";value=1},
			{name="divide";value=0},
			
			{name="cfi_rx";value=-1},
			{name="subframe_rx";value=-1},
			{name="mcs_rx";value=-1},
			{name="nof_rbg_rx";value=-1},
			{name="nof_prb_rx";value=-1},
			{name="sfn_rx";value=-1},
			{name="rbg_mask_rx";value=0},
			
			{name="nof_ports";value=1},
			{name="cell_id";value=0},{name="nof_osymb_x_subf";value=14},
			
			{name="delay_synchro";value=0},
			{name="delay_resdemapp_pbch";value=0},			
			{name="delay_pcfich_rx_descrambling";value=0},
			{name="delay_resdemapp_pdcch";value=0},			
			{name="delay_pdsch_rx_demodulator";value=0},
			{name="delay_pdsch_rx_descrambling";value=0},
			{name="delay_pdsch_rx_unratematching";value=0},
			{name="delay_resdemapp_pdsch";value=0}
			
		);
	};
		
	source:
	{
		binary="modrep_default/libsource.so";	
		mopts=6;
		variables=(
			{name="block_length";value=100},{name="generator";value=0}
		);
	};

	pbch_tx:
	{
		include="./pbch_tx.app";	
	};
	
	pcfich_tx:
	{
		include="./pcfich_tx.app";	
	};

	pdcch_tx:
	{
		include="./pdcch_tx.app";	
	};
	
	pdsch_tx:
	{
		include="./pdsch_tx.app";	
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=20;
		variables=(
			{name="subframe_idx";value=0},
			{name="nof_pdcch";value=1},{name="pdcch_nofcce_0";value=2}
		);
	};
	
	demux_tx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=9;
		variables=(
			{name="nof_outputs";value=14;},{name="data_type";value=2;});
	};
	
	symb_tx:
	{
		include="./symb_tx.app";	
	};

	channel:
	{
		binary="modrep_default/libchannel.so";
		mopts=7;
		variables=(
			{name="variance";value=0.0},{name="gain_re";value=-1.0},{name="gain_im";value=1.0},
		/*	{name="snr_min";value=3.0},{name="snr_max";value=9.0},{name="snr_step";value=0.1}, 
			{name="num_realizations";value=10000},*/
			{name="noise_scale";value=1.778}
		);		
	};

	synchro:
	{
		binary="modrep_osld/liblte_synchG.so";
		mopts=5;
		variables=({name="bypass";value=1},{name="FFTsize";value=128},
		{name="LTEframe_structtype";value=1;});
	};
	
	symb_rx:
	{
		include="./symb_rx.app";	
	};
	
	mux_rx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=11;
		variables=({name="nof_inputs";value=14;},{name="data_type";value=2;});
	};
		
	equalizer:
	{
		binary="modrep_osld/liblte_equalizer.so";
		mopts=5;
	};
	
	resdemapp_pbch:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=17;
		variables=(
			{name="channel_id_0";value=6},
			{name="nof_prb";value=6},{name="fft_size";value=128}, /* initially sample at 1.9 MHz, then whatever */
			{name="nof_osymb_x_subf";value=14}, /* should be received from synch */
			{name="subframe_idx";value=-1},{name="cfi";value=-1});
	};
	
	pbch_rx:
	{
		include="./pbch_rx.app";	
	};

	resdemapp_pcfich:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=18;
		variables=(
			{name="channel_id_0";value=4},{name="subframe_idx";value=-1});
	};
	
	pcfich_rx:
	{
		include="./pcfich_rx.app";	
	};
	
	resdemapp_pdcch:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=31;
		variables=(
			{name="channel_id_0";value=2},{name="subframe_idx";value=-1});
	};
	
	pdcch_rx:
	{
		include="./pdcch_rx.app";	
	};
	
	resdemapp_pdsch:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=12;
		variables=(
			{name="channel_id_0";value=0},{name="subframe_idx";value=-1});
	};
	
	pdsch_rx:
	{
		include="./pdsch_rx.app";	
	};
	
	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=4;
		variables=({name="is_complex";value=1},{name="mode";value=0});
	};	
	
	
};

join_stages=
(
	("ctrl_mux","ctrl","source","pbch_tx_pack","pbch_tx_crc","pbch_tx_pack","pbch_tx_crc_scramble","pbch_tx_coder","pbch_tx_ratematching","pbch_tx_demux","pbch_tx_scrambling","pbch_tx_modulator","pcfich_tx_coder","pcfich_tx_modulator","pcfich_tx_scrambling","pdcch_tx_pack","pdcch_tx_crc","pdcch_tx_coder","pdcch_tx_ratematching","pdcch_tx_scrambling","pdcch_tx_modulator","pdsch_tx_crc_tb","pdsch_tx_coder","pdsch_tx_ratematching","pdsch_tx_scrambling","pdsch_tx_modulator","resmapp","demux_tx"),
	("symb_tx_ifft_0","symb_tx_cyclic_first_0"),
	("symb_tx_ifft_1","symb_tx_cyclic_0"),
	("symb_tx_ifft_2","symb_tx_cyclic_1"),
	("symb_tx_ifft_3","symb_tx_cyclic_2"),
	("symb_tx_ifft_4","symb_tx_cyclic_3"),
	("symb_tx_ifft_5","symb_tx_cyclic_4"),
	("symb_tx_ifft_6","symb_tx_cyclic_5"),
	("symb_tx_ifft_7","symb_tx_cyclic_first_1"),
	("symb_tx_ifft_8","symb_tx_cyclic_6"),
	("symb_tx_ifft_9","symb_tx_cyclic_7"),
	("symb_tx_ifft_10","symb_tx_cyclic_8"),
	("symb_tx_ifft_11","symb_tx_cyclic_9"),
	("symb_tx_ifft_12","symb_tx_cyclic_10"),
	("symb_tx_ifft_13","symb_tx_cyclic_11"),
	
	("channel","synchro"),
	
	("symb_rx_remcyclic_first_0","symb_rx_fft_0"),
	("symb_rx_remcyclic_0","symb_rx_fft_1"),
	("symb_rx_remcyclic_1","symb_rx_fft_2"),
	("symb_rx_remcyclic_2","symb_rx_fft_3"),
	("symb_rx_remcyclic_3","symb_rx_fft_4"),
	("symb_rx_remcyclic_4","symb_rx_fft_5"),
	("symb_rx_remcyclic_5","symb_rx_fft_6"),
	("symb_rx_remcyclic_first_1","symb_rx_fft_7"),
	("symb_rx_remcyclic_6","symb_rx_fft_8"),
	("symb_rx_remcyclic_7","symb_rx_fft_9"),
	("symb_rx_remcyclic_8","symb_rx_fft_10"),
	("symb_rx_remcyclic_9","symb_rx_fft_11"),
	("symb_rx_remcyclic_10","symb_rx_fft_12"),
	("symb_rx_remcyclic_11","symb_rx_fft_13"),

	("mux_rx","equalizer","resdemapp_pbch","pbch_rx_demodulator","pbch_rx_descrambling","pbch_rx_unratematching","pbch_rx_decoder","pbch_rx_crc_descramble","pbch_rx_crc_check","pbch_rx_unpack"),	
	("resdemapp_pcfich","pcfich_rx_demodulation","pcfich_rx_descrambling","pcfich_rx_decoder"),
	("resdemapp_pdcch","pdcch_rx_descrambling","pdcch_rx_demodulator","pdcch_rx_unratematching","pdcch_rx_decoder","pdcch_rx_crc_check","pdcch_rx_unpack"),
	("resdemapp_pdsch","pdsch_rx_descrambling","pdsch_rx_demodulator","pdsch_rx_unratematching","pdsch_rx_decoder","pdsch_rx_uncrc_tb","sink")
);

interfaces:
(
	{src="ctrl_mux";dest="ctrl";delay=0},
	
	{src="source";dest="pdsch_tx"},
	{src="pdsch_tx";dest=("resmapp",0)},
	{src="pcfich_tx";dest=("resmapp",1)},
	{src="pdcch_tx";dest=("resmapp",2)},
	{src="pbch_tx";dest=("resmapp",3)},
		
	{src="resmapp";dest="demux_tx"},
	
	{src=("demux_tx",0);dest=("symb_tx",0)},	
	{src=("demux_tx",1);dest=("symb_tx",1)},	
	{src=("demux_tx",2);dest=("symb_tx",2)},	
	{src=("demux_tx",3);dest=("symb_tx",3)},	
	{src=("demux_tx",4);dest=("symb_tx",4)},	
	{src=("demux_tx",5);dest=("symb_tx",5)},	
	{src=("demux_tx",6);dest=("symb_tx",6)},	
	{src=("demux_tx",7);dest=("symb_tx",7)},	
	{src=("demux_tx",8);dest=("symb_tx",8)},	
	{src=("demux_tx",9);dest=("symb_tx",9)},	
	{src=("demux_tx",10);dest=("symb_tx",10)},	
	{src=("demux_tx",11);dest=("symb_tx",11)},	
	{src=("demux_tx",12);dest=("symb_tx",12)},	
	{src=("demux_tx",13);dest=("symb_tx",13)},	

	{src="symb_tx";dest="channel"},	

	{src="channel";dest="synchro"},
	{src="synchro";dest="symb_rx"},

	/* loop back to control */
	{src=("synchro",1);dest=("ctrl_mux",0);mbpts=0.0;delay=3},
	
	{src=("symb_rx",0);dest=("mux_rx",0)},
	{src=("symb_rx",1);dest=("mux_rx",1)},
	{src=("symb_rx",2);dest=("mux_rx",2)},
	{src=("symb_rx",3);dest=("mux_rx",3)},
	{src=("symb_rx",4);dest=("mux_rx",4)},
	{src=("symb_rx",5);dest=("mux_rx",5)},
	{src=("symb_rx",6);dest=("mux_rx",6)},
	{src=("symb_rx",7);dest=("mux_rx",7)},
	{src=("symb_rx",8);dest=("mux_rx",8)},
	{src=("symb_rx",9);dest=("mux_rx",9)},
	{src=("symb_rx",10);dest=("mux_rx",10)},
	{src=("symb_rx",11);dest=("mux_rx",11)},
	{src=("symb_rx",12);dest=("mux_rx",12)},
	{src=("symb_rx",13);dest=("mux_rx",13)},
	
	{src="mux_rx";dest="equalizer"},	
	{src="equalizer";dest="resdemapp_pbch"},	

	{src=("resdemapp_pbch",0);dest="pbch_rx"},
	{src=("resdemapp_pbch",1);dest="resdemapp_pcfich"},

	/* loop back to control */
	{src="pbch_rx";dest=("ctrl_mux",1);mbpts=0.0;delay=0},

	{src=("resdemapp_pcfich",0);dest="pcfich_rx"},	
	{src=("resdemapp_pcfich",1);dest="resdemapp_pdcch"},

	/* loop back to control */
	{src="pcfich_rx";dest=("ctrl_mux",2);mbpts=0.0;delay=0},

	{src=("resdemapp_pdcch",0);dest="pdcch_rx"},	
	{src=("resdemapp_pdcch",1);dest="resdemapp_pdsch"},

	/* loop back to control */
	{src="pdcch_rx";dest=("ctrl_mux",3);mbpts=0.0;delay=0},
	
	{src=("resdemapp_pdsch",0);dest="pdsch_rx"},	
	
	{src="pdsch_rx";dest="sink"}
);


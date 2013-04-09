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
		mopts=12;
		variables=(
			{name="nof_inputs";value=2;},{name="data_type";value=0;});
	};
	
	ctrl:
	{
		binary="modrep_osld/liblte_ctrl.so";	
		mopts=0.1;
		variables=(
			{name="nof_output_data_itf";value=1},
			{name="mode";value=2}, /* 0 tx, 1 rx, 2 both */
			{name="mcs";value=0},{name="nrb";value=4},{name="fft_size";value=128},
			
			{name="cfi_tx";value=1},{name="cfi_rx";value=-1},
			{name="subframe_rx";value=-1},
			
			{name="nof_ports";value=1},
			{name="nof_prb";value=6;},
			{name="cell_id";value=0},{name="nof_osymb_x_subf";value=14},

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
		mopts=8;
		variables=(
			{name="block_length";value=120},{name="generator";value=0}
		);
	};
	
	pcfich_tx:
	{
		include="../aloe_git/pcfich_tx.app";	
	};

	pdcch_tx:
	{
		include="../aloe_git/pdcch_tx.app";	
	};
	
	pdsch_tx:
	{
		include="../aloe_git/pdsch_tx.app";	
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=15;
		variables=(
			{name="subframe_idx";value=0},
			{name="nof_pdcch";value=1},{name="pdcch_nofcce_0";value=2}
		);
	};
	
	demux_tx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=12;
		variables=(
			{name="nof_outputs";value=14;},{name="data_type";value=2;});
	};
	
	symb_tx:
	{
		include="../aloe_git/symb_tx.app";	
	};

	channel:
	{
		binary="modrep_default/libgen_channel.so";
		mopts=1000;
		variables=(
			{name="variance";value=0.0},{name="gain_re";value=1.0},{name="gain_im";value=0.0},
		/*	{name="snr_min";value=3.0},{name="snr_max";value=9.0},{name="snr_step";value=0.1}, 
			{name="num_realizations";value=10000},*/
			{name="noise_scale";value=1.778}
		);		
	};

	synchro:
	{
		binary="modrep_osld/liblte_synchG.so";
		mopts=100;
		variables=({name="bypass";value=0},{name="FFTsize";value=128},
		{name="LTEframe_structtype";value=1;});
	};

	symb_rx:
	{
		include="../aloe_git/symb_rx.app";	
	};
	
	mux_rx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=11;
		variables=({name="nof_inputs";value=14;},{name="data_type";value=2;});
	};
	
	resdemapp_pcfich:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=12;
		variables=(
			{name="channel_id_0";value=4},{name="subframe_idx";value=-1});
	};
	
	pcfich_rx:
	{
		include="../aloe_git/pcfich_rx.app";	
	};
	
	resdemapp_pdcch:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=12;
		variables=(
			{name="channel_id_0";value=2},{name="subframe_idx";value=-1});
	};
	
	pdcch_rx:
	{
		include="../aloe_git/pdcch_rx.app";	
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
		include="../aloe_git/pdsch_rx.app";	
	};
	
	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=5;
		variables=({name="is_complex";value=1},{name="mode";value=0});
	};	
	
};

join_stages=
(
	("ctrl_mux","ctrl","source","pcfich_tx_coder","pcfich_tx_modulator","pcfich_tx_scrambling","pdcch_tx_crc","pdcch_tx_coder","pdcch_tx_ratematching","pdcch_tx_scrambling","pdcch_tx_modulator","pdsch_tx_crc_tb","pdsch_tx_coder","pdsch_tx_ratematching","pdsch_tx_scrambling","pdsch_tx_modulator","resmapp","demux_tx"),
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

	("mux_rx","resdemapp_pcfich","pcfich_rx_demodulation","pcfich_rx_descrambling","pcfich_rx_decoder"),
	("resdemapp_pdcch","pdcch_rx_descrambling","pdcch_rx_demodulator","pdcch_rx_unratematching","pdcch_rx_decoder","pdcch_rx_crc_check"),
	("resdemapp_pdsch","pdsch_rx_descrambling","pdsch_rx_demodulator","pdsch_rx_unratematching","pdsch_rx_decoder","pdsch_rx_uncrc_tb","sink")
);

interfaces:
(
	{src="ctrl_mux";dest="ctrl";delay=0},
	
	{src="ctrl";dest="pdcch_tx"},
	
	{src="source";dest="pdsch_tx"},
	{src="pdsch_tx";dest=("resmapp",0)},
	{src="pcfich_tx";dest=("resmapp",1)},
	{src="pdcch_tx";dest=("resmapp",2)},
		
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
	
	{src="mux_rx";dest="resdemapp_pcfich"},	
	
	{src=("resdemapp_pcfich",0);dest="pcfich_rx"},	
	{src=("resdemapp_pcfich",1);dest="resdemapp_pdcch"},

	{src=("resdemapp_pdcch",0);dest="pdcch_rx"},	
	{src=("resdemapp_pdcch",1);dest="resdemapp_pdsch"},

	{src=("resdemapp_pdsch",0);dest="pdsch_rx"},	

	/* loop back to control */
	{src="pcfich_rx";dest=("ctrl_mux",1);mbpts=0.0;delay=0},
	
	{src="pdsch_rx";dest="sink"}
);


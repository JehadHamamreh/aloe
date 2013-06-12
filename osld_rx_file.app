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
	rx:
	{
		binary="modrep_default/libfile_source.so";	
		mopts=13.0;
		log=false;

		variables=({name="file_name";value="output.bin"},
				{name="block_length";value=15360});
	};
	

	/* muxes all control messages from other modules */
	ctrl_mux: 
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=4.0;
		log=false;
		variables=(
			{name="nof_inputs";value=4;},{name="data_type";value=0;});
	};
	
	ctrl:
	{
		binary="modrep_osld/liblte_ctrl.so";	
		mopts=35.0;
		log=true;		
		variables=(
			{name="nof_output_data_itf";value=0},
			{name="mode";value=1}, /* 0 tx, 1 rx, 2 both */
			{name="mcs_tx";value=9},
			{name="nof_rbg_tx";value=6},
			{name="rbg_mask_tx";value=0x3f},
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
			
			{name="delay_synchro_pss";value=0},
			{name="delay_synchro_sss";value=0},
			{name="delay_equalizer";value=0},			
			{name="delay_resdemapp_pbch";value=0},			
			{name="delay_pbch_rx_descrambling";value=0},			
			{name="delay_resdemapp_pcfich";value=0},			
			{name="delay_resdemapp_pdsch";value=0},			
			{name="delay_pcfich_rx_descrambling";value=0},
			{name="delay_resdemapp_pdcch";value=0},			
			{name="delay_pdcch_rx_descrambling";value=0},			
			{name="delay_pdsch_rx_demodulator";value=0},
			{name="delay_pdsch_rx_descrambling";value=0},
			{name="delay_pdsch_rx_unratematching";value=0},
			{name="delay_resdemapp_pdsch";value=0}
			
		);
	};
	
	agc:
	{
		binary="modrep_osld/libgen_agc.so";	
		mopts=27.0;
		log=false;
		variables=(
			//{name="power";value=742.0}
			{name="scale";value=1.0}
		);
	};
		
	synchro_pss:
	{
		binary="modrep_osld/liblte_pss_synch.so";
		mopts=266.0;
		log=true;
		variables=(
			{name="bypass";value=0},
			{name="unsync_nof_pkts";value=0},
			{name="do_cfo";value=1},
			{name="correlation_threshold";value=10000},
			{name="input_len";value=1920},{name="N_id_2";value=0});
	};

	logtime_st: 
	{
		binary="modrep_default/liblogtime.so";	
		mopts=8.0;
		log=false;
		variables=({name="bypass";value=1},
				{name="file_name";value="time_st.log";},{name="log_size";value=100000});
	};
	
	dup:
	{
		binary="modrep_default/libdup.so";
		mopts=11.0;
		log=false;
		variables=({name="nof_output_itf";value=2});
	};

	synchro_sss:
	{
		binary="modrep_osld/liblte_sss_synch.so";
		mopts=52.0;
		log=true;
		variables=(
			{name="correlation_threshold";value=3000},
			{name="input_len";value=1920},{name="N_id_2";value=0});
	};

	symb_rx:
	{
		include="./symb_rx.app";	
	};
	
	mux_rx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=11.0;
		log=false;
		variables=({name="nof_inputs";value=14;},{name="check_all";value=1},{name="data_type";value=2;});
	};

	equalizer:
	{
		binary="modrep_osld/liblte_equalizer.so";	
		mopts=27.0;
		log=false;
		variables=({name="bypass";value=0},{name="ntime";value=4},{name="nfreq";value=10});
	};

	resdemapp_pbch:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=19.0;
		log=false;		
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
		mopts=22;
		log=false;		
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
		mopts=29.0;
		log=false;		
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
		mopts=22.0;
		log=false;		
		variables=(
			{name="channel_id_0";value=0},{name="subframe_idx";value=-1});
	};
	
	pdsch_rx:
	{
		include="./pdsch_rx.app";	
	};
		
	logtime_end: 
	{
		binary="modrep_default/liblogtime.so";	
		mopts=4.0;
		log=false;
		variables=({name="bypass";value=1},
				{name="file_name";value="time_end.log";},{name="log_size";value=100000});
	};
	
	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=3;
		variables=({name="is_complex";value=1},{name="mode";value=0});
	};	
	
	
};

join_stages=
(
	("rx","ctrl_mux","ctrl","agc","synchro_pss","logtime_st","dup","synchro_sss"),
	
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
	("resdemapp_pdsch","pdsch_rx_demodulator","pdsch_rx_descrambling","pdsch_rx_unratematching","pdsch_rx_decoder","pdsch_rx_uncrc_tb","logtime_end","sink")
);

interfaces:
(
	{src="ctrl_mux";dest="ctrl";delay=0},
	
	{src="rx";dest="agc"},
	{src="agc";dest="synchro_pss"},
	{src="synchro_pss";dest="logtime_st"},
	{src="logtime_st";dest="dup"},
	{src=("dup",0);dest="synchro_sss"},
	{src=("dup",1);dest="symb_rx"},

	/* loop back to control */
	{src="synchro_sss";dest=("ctrl_mux",0);mbpts=0.0;delay=3},
	
	{src=("symb_rx",0);dest=("mux_rx",0);log=false},
	{src=("symb_rx",1);dest=("mux_rx",1);log=false},
	{src=("symb_rx",2);dest=("mux_rx",2);log=false},
	{src=("symb_rx",3);dest=("mux_rx",3);log=false},
	{src=("symb_rx",4);dest=("mux_rx",4);log=false},
	{src=("symb_rx",5);dest=("mux_rx",5);log=false},
	{src=("symb_rx",6);dest=("mux_rx",6);log=false},
	{src=("symb_rx",7);dest=("mux_rx",7);log=false},
	{src=("symb_rx",8);dest=("mux_rx",8);log=false},
	{src=("symb_rx",9);dest=("mux_rx",9);log=false},
	{src=("symb_rx",10);dest=("mux_rx",10);log=false},
	{src=("symb_rx",11);dest=("mux_rx",11);log=false},
	{src=("symb_rx",12);dest=("mux_rx",12);log=false},
	{src=("symb_rx",13);dest=("mux_rx",13);log=false},
	
	
	{src="mux_rx";dest="equalizer"},	
	{src="equalizer";dest="resdemapp_pbch"},	

	{src=("resdemapp_pbch",0);dest="pbch_rx"},
	{src=("resdemapp_pbch",1);dest="resdemapp_pcfich"},

	/* loop back to control */
	{src="pbch_rx";dest=("ctrl_mux",1);mbpts=0.0;delay=1},

	{src=("resdemapp_pcfich",0);dest="pcfich_rx"},	
	{src=("resdemapp_pcfich",1);dest="resdemapp_pdcch"},

	/* loop back to control */
	{src="pcfich_rx";dest=("ctrl_mux",2);mbpts=0.0;delay=1},

	{src=("resdemapp_pdcch",0);dest="pdcch_rx"},	
	{src=("resdemapp_pdcch",1);dest="resdemapp_pdsch"},

	/* loop back to control */
	{src="pdcch_rx";dest=("ctrl_mux",3);mbpts=0.0;delay=1},
	
	{src=("resdemapp_pdsch",0);dest="pdsch_rx"},	
	
	{src="pdsch_rx";dest="logtime_end"},
	{src="logtime_end";dest="sink"}
);


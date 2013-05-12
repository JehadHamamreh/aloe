main:
{
	/* if selected, each module is called waveform_granularity_us/time_slot times per timeslot */
	/* if set to non-zero, the platform time slot must be integer divisible of waveform_granularity_us */
	waveform_granularity_us=0;
	
	precach_pipeline=true;
	auto_ctrl_module="ctrl";
};

modules:
{
	rx:
	{
		binary="modrep_default/libfile_source.so";	
		mopts=8;
		/*
		variables=({name="address";value="0.0.0.0"},{name="port";value=10000;}
		*/
		variables=({name="file_name";value="output.bin"}
				,{name="block_length";value=15360}
			); 
	};

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
			{name="mode";value=1}, /* 0 tx, 1 rx, 2 both */
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
	("ctrl_mux","ctrl","rx","synchro"),
	
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

	("mux_rx","resdemapp_pbch","pbch_rx_demodulator","pbch_rx_descrambling","pbch_rx_unratematching","pbch_rx_decoder","pbch_rx_crc_descramble","pbch_rx_crc_check","pbch_rx_unpack"),	
	("resdemapp_pcfich","pcfich_rx_demodulation","pcfich_rx_descrambling","pcfich_rx_decoder"),
	("resdemapp_pdcch","pdcch_rx_descrambling","pdcch_rx_demodulator","pdcch_rx_unratematching","pdcch_rx_decoder","pdcch_rx_crc_check","pdcch_rx_unpack"),
	("resdemapp_pdsch","pdsch_rx_descrambling","pdsch_rx_demodulator","pdsch_rx_unratematching","pdsch_rx_decoder","pdsch_rx_uncrc_tb","sink")
);

interfaces:
(
	{src="ctrl_mux";dest="ctrl";delay=0},
	
	{src="rx";dest="synchro"},
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
	
	{src="mux_rx";dest="resdemapp_pbch"},	

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


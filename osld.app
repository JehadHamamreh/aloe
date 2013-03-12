main:
{
	/* if selected, each module is called waveform_granularity_us/time_slot times per timeslot */
	/* if set to non-zero, the platform time slot must be integer divisible of waveform_granularity_us */
	waveform_granularity_us=0;
	
	precach_pipeline=false;
	auto_ctrl_module="ctrl_tx";
};

modules:
{
	ctrl_tx:
	{
		binary="modrep_osld/liblte_ctrl_tx.so";	
		mopts=0.1;
		variables=(
			{name="mcs";value=0},{name="nrb";value=4},{name="fft_size";value=128},
			{name="cp_is_long";value=0;}
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
		include="./pcfich_tx.app";	
	};
	

	pdsch_tx:
	{
		include="./pdsch_tx.app";	
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=15;
		variables=(
			{name="direction";value=0;},{name="subframe_idx";value=0});
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
		include="./symb_tx.app";	
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
	
	resdemapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=12;
		variables=(
			{name="direction";value=1;},{name="subframe_idx";value=0});
	};
	
	pcfich_rx:
	{
		include="./pcfich_rx.app";	
	};
	
	
	pdsch_rx:
	{
		include="./pdsch_rx.app";	
	};
	
	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=5;
		variables=({name="is_complex";value=1},{name="mode";value=0});
	};	
	
};


interfaces:
(
	{src=("source",0);dest=("pdsch_tx",0)},
	{src="pdsch_tx";dest=("resmapp",0)},
	{src="pcfich_tx";dest=("resmapp",1)},
		
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

	{src="channel";dest="symb_rx"},
	
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
	
	{src="mux_rx";dest="resdemapp"},	
	
	{src=("resdemapp",0);dest="pdsch_rx"},	
	{src=("resdemapp",1);dest="pcfich_rx"},	
	
	{src="pdsch_rx";dest="sink"}
);


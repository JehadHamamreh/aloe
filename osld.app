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
			{name="nof_inputs";value=1;},{name="data_type";value=0;});
	};
	
	ctrl:
	{
		binary="modrep_osld/liblte_ctrl.so";	
		mopts=0.1;
		variables=(
			{name="mode";value=2}, /* 0 tx, 1 rx, 2 both */
			{name="mcs";value=0},{name="nrb";value=4},{name="fft_size";value=128},
			
			{name="cfi_tx";value=1},{name="cfi_rx";value=-1},
			
			{name="nof_ports";value=1},
			{name="nof_prb";value=6;},
			{name="nof_pdsch";value=1},{name="pdsch_rbgmask_0";value=0xffff},
			{name="cell_id";value=0},{name="nof_osymb_x_subf";value=14},
			{name="itf_77_delay";value=4}
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
	
	pdsch_tx:
	{
		include="../aloe_git/pdsch_tx.app";	
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=15;
		variables=(
			{name="subframe_idx";value=0});
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
			{name="channel_id_0";value=4},{name="subframe_idx";value=0});
	};
	
	pcfich_rx:
	{
		include="../aloe_git/pcfich_rx.app";	
	};
	
	resdemapp_pdsch:
	{
		binary="modrep_osld/liblte_resource_demapper.so";	
		mopts=12;
		variables=(
			{name="cfi";value=-1},{name="channel_id_0";value=0},{name="subframe_idx";value=0});
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


interfaces:
(
	{src="ctrl_mux";dest="ctrl";delay=0},
	
	{src="source";dest="pdsch_tx"},
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
	
	{src="mux_rx";dest="resdemapp_pcfich"},	
	
	{src=("resdemapp_pcfich",0);dest="pcfich_rx"},	
	{src=("resdemapp_pcfich",1);dest="resdemapp_pdsch";delay=22},	
	{src=("resdemapp_pdsch",0);dest="pdsch_rx"},	

	/* loop back to control */
	{src="pcfich_rx";dest="ctrl_mux";mbpts=0.0;delay=0},
	
	{src="pdsch_rx";dest="sink"}
);


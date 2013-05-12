
modules:
{	
	source:
	{
		binary="modrep_default/libsource.so";	
		mopts=6;
		variables=(
			{name="block_length";value=1000},
			{name="generator";value=0});		/* 0: bits, 4: BPSK complex */
	};

	pusch_tx:
	{
		include="./pusch_tx.app";	
	};

	/* zero padding (to be substituted by resource_mapping */ 
	padding:
	{
		binary="modrep_osld/libgen_padding.so";	
		mopts=5;
		variables=(
			{name="direction";value=0},	/* padding */
			{name="data_type";value=0},	/* 0: _Complex float, 1: real, 2: bits */
			{name="pre_padding";value=28},{name="post_padding";value=28},
			{name="nof_packets";value=14});
	};

	demux_tx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=9;
		variables=(
			{name="nof_outputs";value=14;},
			{name="data_type";value=2;}); 	/* 0: bits, 1: real, 2: complex*/
	};
	
	symb_tx:
	{
		include="./symb_ul_tx.app";	
	};


	channel:
	{
		binary="modrep_default/libchannel.so";
		mopts=7;
		variables=(
			{name="variance";value=0.0},{name="gain_re";value=1.0},{name="gain_im";value=0.0},
			{name="noise_scale";value=1.778}
		);		
	};

	symb_rx:
	{
		include="./symb_ul_rx.app";	
	};
	
	mux_rx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=11;
		variables=(
			{name="nof_inputs";value=14;},
			{name="data_type";value=2;});
	};

	/* unpadding (to be substituted by resource_demapping) */ 
	unpadding:
	{
		binary="modrep_osld/libgen_padding.so";	
		mopts=5;
		variables=(
			{name="direction";value=1},	/* unpadding */
			{name="data_type";value=0},	/* _Complex float */
			{name="pre_padding";value=28},{name="post_padding";value=28},
			{name="nof_packets";value=14});
	};


	pusch_rx:
	{
		include="./pusch_rx.app";	
	};
	
	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=4;
		variables=({name="is_complex";value=1},{name="mode";value=0});
	};	
	
};

interfaces:
(
	{src="source";dest="pusch_tx"},
	{src="pusch_tx";dest="padding"},
	{src="padding";dest="demux_tx"},

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

	{src="mux_rx";dest="unpadding"},
	{src="unpadding";dest="pusch_rx"},	
	{src="pusch_rx";dest="sink"}
);


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

	/* bit-level processing */
	pusch_tx:
	{
		include="./pusch_tx.app";	
	};

	dft:
	{
		binary="modrep_osld/libgen_dft.so";	
		mopts=10;
		instances=1;
		variables=(
			{name="dc_offset";value=0},
			{name="direction";value=0},{name="mirror";value=0},{name="normalize";value=1},
			{name="dft_size";value=72},{name="psd";value=0},{name="out_db";value=0}
		);
	}

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

	synchro:
	{
		binary="modrep_osld/liblte_synchG.so";
		mopts=5;
		variables=({name="bypass";value=1},{name="FFTsize";value=128},
		{name="LTEframe_structtype";value=1;});
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

	idft:
	{
		binary="modrep_osld/libgen_dft.so";	
		mopts=100;
		instances=1;
		variables=(
			{name="dc_offset";value=0},
			{name="direction";value=1;},{name="mirror";value=0;},{name="normalize";value=1;},
			{name="dft_size";value=72;},{name="psd";value=0},{name="out_db";value=0}
		);
	};

	/* bit-level processing */
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

join_stages=
(
	("source","pusch_tx_crc_tb","pusch_tx_coder","pusch_tx_ratematching","pusch_tx_scrambling","pusch_tx_modulator"),

	("dft","padding","demux_tx"),

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

	("mux_rx","unpadding","idft"),	
	("pusch_rx_demodulator","pusch_rx_descrambling","pusch_rx_unratematching","pusch_rx_decoder","pusch_rx_uncrc_tb","sink")
);

interfaces:
(
	{src="source";dest="pusch_tx"},
	{src="pusch_tx";dest="dft"},
	{src="dft";dest="padding"},
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
	{src="channel";dest="synchro"},
	{src="synchro";dest="symb_rx"},

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
	{src="unpadding";dest="idft"},
	{src="idft";dest="pusch_rx"},	
	{src="pusch_rx";dest="sink"}
);


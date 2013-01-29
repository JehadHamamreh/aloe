modules:
{
	source:
	{
		binary="modrep_osld/liblte_tb_source.so";	
		mopts=23;
		stage=1;
		variables=(
			{name="mcs";value=0},{name="nrb";value=4;}
		);
	};

	crc_tb:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=17.8;
		stage=2;
		variables=({name="long_crc";value=24;});
	};	

	coder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=6;
		instances=3;
		stage=3;
		variables=({name="direction";value=0},{name="padding";value=0;});
	};
	
	ratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=40;
		stage=4;
		variables=(
			{name="direction";value=0},{name="mcs";value=0},{name="nrb";value=4;},
			{"fft_size";value=128;},{"cp_is_long";value=0;},{"lteslots_x_timeslot";value=2;},{name="rvidx";value=0;}
		);
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=30;
		stage=5;
		variables=({name="modulation";value=1;);
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=10;
		stage=6;
		variables=(
			{name="direction";value=0;},{"fft_size";value=128;},{"lteslots_x_timeslot";value=2;});
	};
	
	demux_tx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=10;
		stage=7;
		variables=(
			{name="nof_outputs";value=14;},{"data_type";value=2;});
	};

/* there are 14 branches of the following two blocks */
	ifft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=84;
		stage=8;
		instances=14
		variables=(
			{name="direction";value=1;},{name="mirror";value=1;},{name="normalize";value=1;},
			{name="dft_size";value=128;}
		);
	};
	
	cyclic_first:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=28;
		stage=9;
		instances=2
		variables=(
			{name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=10})
		);
	};

	cyclic:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=28;
		stage=9;
		instances=12
		variables=(
			{name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=9})
		);
	};
	
	mux_tx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=10;
		stage=7;
		variables=(
			{name="nof_inputs";value=14;},{"data_type";value=2;});
	};

	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=5;
		stage=11;
		variables=(
			{name="mode";value=3},
			{name="is_complex";value=1}
		);		
	};
};

interfaces:
(
	{src=("source",0);dest=("crc_tb",0)},
	{src="crc_tb";dest="coder"},
	{src="coder";dest="ratematching"},
	{src="ratematching";dest="modulator"},	
	{src="modulator";dest="resmapp"},	
	{src="resmapp";dest="demux"},
	
	{src=("demux",0);dest="fft_0"},	
	{src="fft_0";dst="cyclic_first_0"},
	{src="cyclic_first_0";dst=("mux",0)},

	{src=("demux",1);dest="fft_1"},	
	{src="fft_1";dst="cyclic_0"},
	{src="cyclic_0";dst=("mux",1)},

	{src=("demux",2);dest="fft_2"},	
	{src="fft_2";dst="cyclic_1"},
	{src="cyclic_1";dst=("mux",2)},

	{src=("demux",3);dest="fft_3"},	
	{src="fft_3";dst="cyclic_2"},
	{src="cyclic_2";dst=("mux",3)},
	
	{src=("demux",4);dest="fft_4"},	
	{src="fft_4";dst="cyclic_3"},
	{src="cyclic_3";dst=("mux",4)},
	
	{src=("demux",5);dest="fft_5"},	
	{src="fft_5";dst="cyclic_4"},
	{src="cyclic_4";dst=("mux",5)},

	{src=("demux",6);dest="fft_6"},	
	{src="fft_6";dst="cyclic_5"},
	{src="cyclic_5";dst=("mux",6)},

	{src=("demux",7);dest="fft_7"},	
	{src="fft_7";dst="cyclic_first_1"},
	{src="cyclic_first_1";dst=("mux",7)},

	{src=("demux",8);dest="fft_8"},	
	{src="fft_8";dst="cyclic_6"},
	{src="cyclic_6";dst=("mux",8)},

	{src=("demux",9);dest="fft_9"},	
	{src="fft_9";dst="cyclic_7"},
	{src="cyclic_7";dst=("mux",9)},

	{src=("demux",10);dest="fft_10"},	
	{src="fft_10";dst="cyclic_8"},
	{src="cyclic_8";dst=("mux",10)},

	{src=("demux",11);dest="fft_11"},	
	{src="fft_11";dst="cyclic_9"},
	{src="cyclic_9";dst=("mux",11)},

	{src=("demux",12);dest="fft_12"},	
	{src="fft_12";dst="cyclic_10"},
	{src="cyclic_10";dst=("mux",12)},

	{src=("demux",13);dest="fft_13"},	
	{src="fft_13";dst="cyclic_11"},
	{src="cyclic_11";dst=("mux",13)},
	
	{src="demux";dest="sink"}	
);

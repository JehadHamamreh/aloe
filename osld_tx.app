main:
{
	/* if selected, each module is called waveform_granularity_us/time_slot times per timeslot */
	/* if set to non-zero, the platform time slot must be integer divisible of waveform_granularity_us */
	waveform_granularity_us=0;
	
	precach_pipeline=true;
};

modules:
{
	source:
	{
		binary="modrep_osld/liblte_tb_source.so";	
		mopts=8;
		variables=(
			{name="enabled";value=1},{name="mcs";value=0},{name="nrb";value=4;}
		);
	};

	crc_tb:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=6;
		variables=({name="long_crc";value=16;});
	};	

	coder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=12;
		variables=({name="direction";value=0},{name="padding";value=0;});
	};
	
	ratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=23;
		variables=(
			{name="direction";value=0},{name="mcs";value=0},{name="nrb";value=4;},
			{name="fft_size";value=128;},{name="cp_is_long";value=0;},{name="lteslots_x_timeslot";value=2;},
			{name="rvidx";value=0;}
		);
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=11;
		variables=({name="modulation";value=1;});
	};

	scrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=11;
		variables=({name="q";value=0;},{name="cell_gr";value=2},{name="cell_sec";value=167});
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=15;
		variables=(
			{name="direction";value=0;},{name="fft_size";value=128;},{name="lteslots_x_timeslot";value=2;});
	};
	
	demux_tx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=12;
		variables=(
			{name="nof_outputs";value=14;},{name="data_type";value=2;});
	};

	ifft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=11;
		instances=14
		variables=(
			{name="direction";value=1;},{name="mirror";value=1;},{name="normalize";value=1;},
			{name="dft_size";value=128;},{name="psd";value=0},{name="out_db";value=0}
		);
	};
	
	cyclic_first:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=4;
		instances=2
		variables=( {name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=10});
	};

	cyclic:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=4;
		instances=12
		variables=({name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=9}	);
	};
	
	mux_tx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=11;
		variables=({name="nof_inputs";value=14;},{name="data_type";value=2;});
	};

	dac_tx:
	{
		binary="modrep_default/libdac_sink.so";	
		mopts=5;
		variables=({name="freq_samp";value=1777777.7;});
	};
		
};

join_stages=
(
	("source","crc_tb","coder","ratematching","modulator","scrambling","resmapp","demux_tx"),
	("ifft_0","cyclic_first_0"),
	("ifft_1","cyclic_0"),
	("ifft_2","cyclic_1"),
	("ifft_3","cyclic_2"),
	("ifft_4","cyclic_3"),
	("ifft_5","cyclic_4"),
	("ifft_6","cyclic_5"),
	("ifft_7","cyclic_first_1"),
	("ifft_8","cyclic_6"),
	("ifft_9","cyclic_7"),
	("ifft_10","cyclic_8"),
	("ifft_11","cyclic_9"),
	("ifft_12","cyclic_10"),
	("ifft_13","cyclic_11")
);

interfaces:
(
	{src=("source",0);dest=("crc_tb",0)},
	{src="crc_tb";dest="coder"},
	{src="coder";dest="ratematching"},
	{src="ratematching";dest="scrambling"},	
	{src="scrambling";dest="modulator"},	
	{src="modulator";dest="resmapp"},	
	{src="resmapp";dest="demux_tx"},
	
	{src=("demux_tx",0);dest="ifft_0"},	
	{src="ifft_0";dest="cyclic_first_0"},
	{src="cyclic_first_0";dest=("mux_tx",0)},

	{src=("demux_tx",1);dest="ifft_1"},	
	{src="ifft_1";dest="cyclic_0"},
	{src="cyclic_0";dest=("mux_tx",1)},

	{src=("demux_tx",2);dest="ifft_2"},	
	{src="ifft_2";dest="cyclic_1"},
	{src="cyclic_1";dest=("mux_tx",2)},

	{src=("demux_tx",3);dest="ifft_3"},	
	{src="ifft_3";dest="cyclic_2"},
	{src="cyclic_2";dest=("mux_tx",3)},
	
	{src=("demux_tx",4);dest="ifft_4"},	
	{src="ifft_4";dest="cyclic_3"},
	{src="cyclic_3";dest=("mux_tx",4)},
	
	{src=("demux_tx",5);dest="ifft_5"},	
	{src="ifft_5";dest="cyclic_4"},
	{src="cyclic_4";dest=("mux_tx",5)},

	{src=("demux_tx",6);dest="ifft_6"},	
	{src="ifft_6";dest="cyclic_5"},
	{src="cyclic_5";dest=("mux_tx",6)},

	{src=("demux_tx",7);dest="ifft_7"},	
	{src="ifft_7";dest="cyclic_first_1"},
	{src="cyclic_first_1";dest=("mux_tx",7)},

	{src=("demux_tx",8);dest="ifft_8"},	
	{src="ifft_8";dest="cyclic_6"},
	{src="cyclic_6";dest=("mux_tx",8)},

	{src=("demux_tx",9);dest="ifft_9"},	
	{src="ifft_9";dest="cyclic_7"},
	{src="cyclic_7";dest=("mux_tx",9)},

	{src=("demux_tx",10);dest="ifft_10"},	
	{src="ifft_10";dest="cyclic_8"},
	{src="cyclic_8";dest=("mux_tx",10)},

	{src=("demux_tx",11);dest="ifft_11"},	
	{src="ifft_11";dest="cyclic_9"},
	{src="cyclic_9";dest=("mux_tx",11)},

	{src=("demux_tx",12);dest="ifft_12"},	
	{src="ifft_12";dest="cyclic_10"},
	{src="cyclic_10";dest=("mux_tx",12)},

	{src=("demux_tx",13);dest="ifft_13"},	
	{src="ifft_13";dest="cyclic_11"},
	{src="cyclic_11";dest=("mux_tx",13)},
	
	{src="mux_tx";dest="dac_tx"}
);


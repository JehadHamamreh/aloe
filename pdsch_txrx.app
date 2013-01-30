modules:
{
	source:
	{
		binary="modrep_osld/liblte_tb_source.so";	
		mopts=4.18;
		stage=1;
		variables=(
			{name="mcs";value=0},{name="nrb";value=4;}
		);
	};

	crc_tb:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=5.55;
		stage=2;
		variables=({name="long_crc";value=16;});
	};	

	coder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=9.71;
		stage=3;
		variables=({name="direction";value=0},{name="padding";value=0;});
	};
	
	ratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=21.38;
		stage=4;
		variables=(
			{name="direction";value=0},{name="mcs";value=0},{name="nrb";value=4;},
			{name="fft_size";value=128;},{name="cp_is_long";value=0;},{name="lteslots_x_timeslot";value=2;},
			{name="rvidx";value=0;}
		);
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=12.03;
		stage=5;
		variables=({name="modulation";value=1;});
	};

	resmapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=13.38;
		stage=6;
		variables=(
			{name="direction";value=0;},{name="fft_size";value=128;},{name="lteslots_x_timeslot";value=2;});
	};
	
	demux_tx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=10.05;
		stage=7;
		variables=(
			{name="nof_outputs";value=14;},{name="data_type";value=2;});
	};

	ifft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=9;
		stage=8;
		instances=14
		variables=(
			{name="direction";value=1;},{name="mirror";value=1;},{name="normalize";value=1;},
			{name="dft_size";value=128;},{name="psd";value=0},{name="out_db";value=0}
		);
	};
	
	cyclic_first:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=3;
		stage=9;
		instances=2
		variables=( {name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=10});
	};

	cyclic:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=3;
		stage=9;
		instances=12
		variables=({name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=9}	);
	};
	
	mux_tx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=10.13;
		stage=10;
		variables=({name="nof_inputs";value=14;},{name="data_type";value=2;});
	};

	channel:
	{
		binary="modrep_default/libgen_channel.so";
		mopts=39.66;
		stage=11;
		variables=(
			{name="variance";value=0.0},{name="gain_re";value=1.0},{name="gain_im";value=0.0}
		);		
	};
	
	demux_rx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=10.2;
		stage=12;
		variables=({name="nof_outputs";value=14;},{name="data_type";value=2;},
					{name="out_len_0";value=138},{name="out_len_7";value=138});
	};
	
	remcyclic_first:
	{
		binary="modrep_osld/libgen_remcyclic.so";	
		mopts=3;
		stage=13;
		instances=2
		variables=({name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=10});
	};

	remcyclic:
	{
		binary="modrep_osld/libgen_remcyclic.so";	
		mopts=3;
		stage=14;
		instances=12
		variables=({name="ofdm_symbol_sz";value=128}, {name="cyclic_prefix_sz";value=9}	);
	};
	
	fft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=9;
		stage=15;
		instances=14
		variables=(
			{name="direction";value=0;},{name="mirror";value=2;},{name="normalize";value=1;},
			{name="dft_size";value=128;},{name="psd";value=0},{name="out_db";value=0}
		);
	};
	
	mux_rx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=10.64;
		stage=16;
		variables=({name="nof_inputs";value=14;},{name="data_type";value=2;});
	};
	
	resdemapp:
	{
		binary="modrep_osld/liblte_resource_mapper.so";	
		mopts=11.22;
		stage=17;
		variables=(
			{name="direction";value=1;},{name="fft_size";value=128;},{name="lteslots_x_timeslot";value=2;});
	};
	
	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=60.22;
		stage=18;
		variables=(
			{name="soft";value=1;},{name="modulation";value=1;},{name="sigma2";value=0.1;});
	};
	
	unratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=64.58;
		stage=19;
		variables=(
			{name="direction";value=1},{name="mcs";value=0},{name="nrb";value=4;},
			{name="fft_size";value=128;},{name="cp_is_long";value=0;},{name="lteslots_x_timeslot";value=2;},
			{name="rvidx";value=0;}
		);
	};
	
	decoder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=266.7;
		stage=20;
		variables=({name="direction";value=1},{name="padding";value=0;});
	};

	uncrc_tb:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=17.8;
		stage=21;
		variables=({name="direction";value=1},{name="long_crc";value=16;});
	};	
	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=15.7;
		stage=22;
		variables=({name="is_complex";value=1},{name="mode";value=0});
	};	
	
};

interfaces:
(
	{src=("source",0);dest=("crc_tb",0)},
	{src="crc_tb";dest="coder"},
	{src="coder";dest="ratematching"},
	{src="ratematching";dest="modulator"},	
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
	
	{src="mux_tx";dest="channel"},	
	{src="channel";dest="demux_rx"},
	
	{src=("demux_rx",0);dest="remcyclic_first_0"},	
	{src="remcyclic_first_0";dest="fft_0"},
	{src="fft_0";dest=("mux_rx",0)},

	{src=("demux_rx",1);dest="remcyclic_0"},	
	{src="remcyclic_0";dest="fft_1"},
	{src="fft_1";dest=("mux_rx",1)},

	{src=("demux_rx",2);dest="remcyclic_1"},	
	{src="remcyclic_1";dest="fft_2"},
	{src="fft_2";dest=("mux_rx",2)},

	{src=("demux_rx",3);dest="remcyclic_2"},	
	{src="remcyclic_2";dest="fft_3"},
	{src="fft_3";dest=("mux_rx",3)},
	
	{src=("demux_rx",4);dest="remcyclic_3"},	
	{src="remcyclic_3";dest="fft_4"},
	{src="fft_4";dest=("mux_rx",4)},
	
	{src=("demux_rx",5);dest="remcyclic_4"},	
	{src="remcyclic_4";dest="fft_5"},
	{src="fft_5";dest=("mux_rx",5)},

	{src=("demux_rx",6);dest="remcyclic_5"},	
	{src="remcyclic_5";dest="fft_6"},
	{src="fft_6";dest=("mux_rx",6)},

	{src=("demux_rx",7);dest="remcyclic_first_1"},	
	{src="remcyclic_first_1";dest="fft_7"},
	{src="fft_7";dest=("mux_rx",7)},

	{src=("demux_rx",8);dest="remcyclic_6"},	
	{src="remcyclic_6";dest="fft_8"},
	{src="fft_8";dest=("mux_rx",8)},

	{src=("demux_rx",9);dest="remcyclic_7"},	
	{src="remcyclic_7";dest="fft_9"},
	{src="fft_9";dest=("mux_rx",9)},

	{src=("demux_rx",10);dest="remcyclic_8"},	
	{src="remcyclic_8";dest="fft_10"},
	{src="fft_10";dest=("mux_rx",10)},

	{src=("demux_rx",11);dest="remcyclic_9"},	
	{src="remcyclic_9";dest="fft_11"},
	{src="fft_11";dest=("mux_rx",11)},

	{src=("demux_rx",12);dest="remcyclic_10"},	
	{src="remcyclic_10";dest="fft_12"},
	{src="fft_12";dest=("mux_rx",12)},

	{src=("demux_rx",13);dest="remcyclic_11"},	
	{src="remcyclic_11";dest="fft_13"},
	{src="fft_13";dest=("mux_rx",13)},
	
	{src="mux_rx";dest="resdemapp"},	
	{src="resdemapp";dest="demodulator"},	
	{src="demodulator";dest="unratematching"},	
	{src="unratematching";dest="decoder"},	
	{src="decoder";dest="uncrc_tb"},	
	{src="uncrc_tb";dest="sink"}
);

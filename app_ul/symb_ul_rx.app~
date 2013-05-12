
modules:
{
	/* parallelize serial data stream into 12 or 14 streams per TTI */
	demux_rx:
	{
		binary="modrep_osld/libgen_demux.so";	
		mopts=11;
		variables=(
			{name="nof_outputs";value=14},{name="data_type";value=2},
			{name="out_len_0";value=138},{name="out_len_7";value=138});
			/* Normal CP: OFDM symbols 0 and 7: 128+10 samples for 1.4 MHz LTE mode (128-point ifft) */
	};
	
	/* cyclic removal of 1st and 8th OFDM symbols (1st and 7th in case of extended CP) */
	remcyclic_first:
	{
		binary="modrep_osld/libgen_remcyclic.so";	
		mopts=3;
		instances=2;
		variables=(
			/*{name="dft_size";value=128},*/
			{name="cyclic_prefix_sz";value=10});
	};

	/* cyclic removal of all other OFDM symbols */
	remcyclic:
	{
		binary="modrep_osld/libgen_remcyclic.so";	
		mopts=3;
		instances=12;
		variables=(
			/*{name="dft_size";value=128},*/
			{name="cyclic_prefix_sz";value=9});
	};
	
	fft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=16;
		instances=14;
		variables=(
			{name="dc_offset";value=1},
			{name="direction";value=0},{name="mirror";value=2},{name="normalize";value=1},
			/*{name="dft_size";value=128},*/
			{name="psd";value=0},{name="out_db";value=0},
			{name="df";value=-7500},{name="fs";value=1920000}
		);
	};
	
};


interfaces:
(
	{src="_input";dest="demux_rx"},
	
	{src=("demux_rx",0);dest="remcyclic_first_0"},	
	{src="remcyclic_first_0";dest="fft_0"},
	{src="fft_0";dest=("_output",0)},

	{src=("demux_rx",1);dest="remcyclic_0"},	
	{src="remcyclic_0";dest="fft_1"},
	{src="fft_1";dest=("_output",1)},

	{src=("demux_rx",2);dest="remcyclic_1"},	
	{src="remcyclic_1";dest="fft_2"},
	{src="fft_2";dest=("_output",2)},

	{src=("demux_rx",3);dest="remcyclic_2"},	
	{src="remcyclic_2";dest="fft_3"},
	{src="fft_3";dest=("_output",3)},
	
	{src=("demux_rx",4);dest="remcyclic_3"},	
	{src="remcyclic_3";dest="fft_4"},
	{src="fft_4";dest=("_output",4)},
	
	{src=("demux_rx",5);dest="remcyclic_4"},	
	{src="remcyclic_4";dest="fft_5"},
	{src="fft_5";dest=("_output",5)},

	{src=("demux_rx",6);dest="remcyclic_5"},	
	{src="remcyclic_5";dest="fft_6"},
	{src="fft_6";dest=("_output",6)},

	{src=("demux_rx",7);dest="remcyclic_first_1"},	
	{src="remcyclic_first_1";dest="fft_7"},
	{src="fft_7";dest=("_output",7)},

	{src=("demux_rx",8);dest="remcyclic_6"},	
	{src="remcyclic_6";dest="fft_8"},
	{src="fft_8";dest=("_output",8)},

	{src=("demux_rx",9);dest="remcyclic_7"},	
	{src="remcyclic_7";dest="fft_9"},
	{src="fft_9";dest=("_output",9)},

	{src=("demux_rx",10);dest="remcyclic_8"},	
	{src="remcyclic_8";dest="fft_10"},
	{src="fft_10";dest=("_output",10)},

	{src=("demux_rx",11);dest="remcyclic_9"},	
	{src="remcyclic_9";dest="fft_11"},
	{src="fft_11";dest=("_output",11)},

	{src=("demux_rx",12);dest="remcyclic_10"},	
	{src="remcyclic_10";dest="fft_12"},
	{src="fft_12";dest=("_output",12)},

	{src=("demux_rx",13);dest="remcyclic_11"},	
	{src="remcyclic_11";dest="fft_13"},
	{src="fft_13";dest=("_output",13)}
);


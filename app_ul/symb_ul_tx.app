
modules:
{
	ifft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=16;
		instances=14
		variables=(
			{name="dc_offset";value=1}, 
			{name="direction";value=1},{name="mirror";value=1},{name="normalize";value=1},
			{name="dft_size";value=128}, /* automatically detected if commented */
			{name="psd";value=0},{name="out_db";value=0},
			{name="df";value=7500},{name="fs";value=1920000}
		);
	};
	
	cyclic_first:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=3;
		instances=2
		variables=( 
			{name="dft_size";value=128}, /* automatically detected if commented */
			{name="cyclic_prefix_sz";value=10});
	};

	cyclic:
	{
		binary="modrep_osld/libgen_cyclic.so";	
		mopts=3;
		instances=12
		variables=(
			{name="dft_size";value=128},  /* automatically detected if commented */
			{name="cyclic_prefix_sz";value=9});
	};
	
	mux_tx:
	{
		binary="modrep_osld/libgen_mux.so";	
		mopts=11;
		variables=({name="nof_inputs";value=14},{name="data_type";value=2});
	};
	
};


interfaces:
(
	{src=("_input",0);dest="ifft_0"},	
	{src="ifft_0";dest="cyclic_first_0"},
	{src="cyclic_first_0";dest=("mux_tx",0)},

	{src=("_input",1);dest="ifft_1"},	
	{src="ifft_1";dest="cyclic_0"},
	{src="cyclic_0";dest=("mux_tx",1)},

	{src=("_input",2);dest="ifft_2"},	
	{src="ifft_2";dest="cyclic_1"},
	{src="cyclic_1";dest=("mux_tx",2)},

	{src=("_input",3);dest="ifft_3"},	
	{src="ifft_3";dest="cyclic_2"},
	{src="cyclic_2";dest=("mux_tx",3)},
	
	{src=("_input",4);dest="ifft_4"},	
	{src="ifft_4";dest="cyclic_3"},
	{src="cyclic_3";dest=("mux_tx",4)},
	
	{src=("_input",5);dest="ifft_5"},	
	{src="ifft_5";dest="cyclic_4"},
	{src="cyclic_4";dest=("mux_tx",5)},

	{src=("_input",6);dest="ifft_6"},	
	{src="ifft_6";dest="cyclic_5"},
	{src="cyclic_5";dest=("mux_tx",6)},

	{src=("_input",7);dest="ifft_7"},	
	{src="ifft_7";dest="cyclic_first_1"},
	{src="cyclic_first_1";dest=("mux_tx",7)},

	{src=("_input",8);dest="ifft_8"},	
	{src="ifft_8";dest="cyclic_6"},
	{src="cyclic_6";dest=("mux_tx",8)},

	{src=("_input",9);dest="ifft_9"},	
	{src="ifft_9";dest="cyclic_7"},
	{src="cyclic_7";dest=("mux_tx",9)},

	{src=("_input",10);dest="ifft_10"},	
	{src="ifft_10";dest="cyclic_8"},
	{src="cyclic_8";dest=("mux_tx",10)},

	{src=("_input",11);dest="ifft_11"},	
	{src="ifft_11";dest="cyclic_9"},
	{src="cyclic_9";dest=("mux_tx",11)},

	{src=("_input",12);dest="ifft_12"},	
	{src="ifft_12";dest="cyclic_10"},
	{src="cyclic_10";dest=("mux_tx",12)},

	{src=("_input",13);dest="ifft_13"},	
	{src="ifft_13";dest="cyclic_11"},
	{src="cyclic_11";dest=("mux_tx",13)},
	
	{src="mux_tx";dest="_output"}
);


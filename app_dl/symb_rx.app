
modules:
{

	de_output:
	{
		binary="modrep_osld/libgen_demux.so";	
		log=false;
		mopts=13.0;
		variables=({name="nof_outputs";value=14;},{name="data_type";value=2;},
					{name="out_len_0";value=138},{name="out_len_7";value=138});
	};
	
	remcyclic_first:
	{
		binary="modrep_osld/libgen_remcyclic.so";	
		log=false;
		mopts=4.0;
		instances=2;
		variables=({name="dft_size";value=128}, {name="cyclic_prefix_sz";value=10});
	};

	remcyclic:
	{
		binary="modrep_osld/libgen_remcyclic.so";	
		log=false;
		mopts=3.0;
		instances=12;
		variables=({name="dft_size";value=128}, {name="cyclic_prefix_sz";value=9}	);
	};
	
	fft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=11;
		log=false;
		instances=14;
		variables=(
			{name="dc_offset";value=1;},
			{name="direction";value=0;},{name="mirror";value=2;},{name="normalize";value=1;},
			{name="dft_size";value=128;},{name="psd";value=0},{name="out_db";value=0}
		);
	};
	
};


interfaces:
(
	{src="_input";dest="de_output"},
	
	{src=("de_output",0);dest="remcyclic_first_0"},	
	{src="remcyclic_first_0";dest="fft_0"},
	{src="fft_0";dest=("_output",0)},

	{src=("de_output",1);dest="remcyclic_0"},	
	{src="remcyclic_0";dest="fft_1"},
	{src="fft_1";dest=("_output",1)},

	{src=("de_output",2);dest="remcyclic_1"},	
	{src="remcyclic_1";dest="fft_2"},
	{src="fft_2";dest=("_output",2)},

	{src=("de_output",3);dest="remcyclic_2"},	
	{src="remcyclic_2";dest="fft_3"},
	{src="fft_3";dest=("_output",3)},
	
	{src=("de_output",4);dest="remcyclic_3"},	
	{src="remcyclic_3";dest="fft_4"},
	{src="fft_4";dest=("_output",4)},
	
	{src=("de_output",5);dest="remcyclic_4"},	
	{src="remcyclic_4";dest="fft_5"},
	{src="fft_5";dest=("_output",5)},

	{src=("de_output",6);dest="remcyclic_5"},	
	{src="remcyclic_5";dest="fft_6"},
	{src="fft_6";dest=("_output",6)},

	{src=("de_output",7);dest="remcyclic_first_1"},	
	{src="remcyclic_first_1";dest="fft_7"},
	{src="fft_7";dest=("_output",7)},

	{src=("de_output",8);dest="remcyclic_6"},	
	{src="remcyclic_6";dest="fft_8"},
	{src="fft_8";dest=("_output",8)},

	{src=("de_output",9);dest="remcyclic_7"},	
	{src="remcyclic_7";dest="fft_9"},
	{src="fft_9";dest=("_output",9)},

	{src=("de_output",10);dest="remcyclic_8"},	
	{src="remcyclic_8";dest="fft_10"},
	{src="fft_10";dest=("_output",10)},

	{src=("de_output",11);dest="remcyclic_9"},	
	{src="remcyclic_9";dest="fft_11"},
	{src="fft_11";dest=("_output",11)},

	{src=("de_output",12);dest="remcyclic_10"},	
	{src="remcyclic_10";dest="fft_12"},
	{src="fft_12";dest=("_output",12)},

	{src=("de_output",13);dest="remcyclic_11"},	
	{src="remcyclic_11";dest="fft_13"},
	{src="fft_13";dest=("_output",13)}
);


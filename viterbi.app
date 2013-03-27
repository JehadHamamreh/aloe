modules:
{
	
	source:
	{
		binary="modrep_default/libsource.so";	
		mopts=8;
		variables=(
			{name="block_length";value=20},{name="generator";value=0}
		);
	};

	
	crc:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=6;
		variables=({name="long_crc";value=16;});
	};	
	
	coder:
	{
		binary="modrep_osld/libgen_convcoder.so";	
		mopts=8;
		variables=(
			{name="constraint_length";value=7},{name="rate";value=3},{name="tail_bit";value=1},
			{name="generator_0";value=91},{name="generator_1";value=127},{name="generator_2";value=117}
		);
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=11;
		variables=({name="modulation";value=2;});
	};
		
	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=50;
		variables=(
			{name="soft";value=1;},{name="modulation";value=2;},{name="sigma2";value=1.5;});
	};
	
	decoder:
	{
		binary="modrep_osld/libgen_viterbi.so";	
		mopts=50;
		variables=(
			{name="constraint_length";value=7},{name="rate";value=3},
			{name="generator_0";value=91},{name="generator_1";value=127},{name="generator_2";value=117}
			);
	};

	crc_check:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=5;
		variables=({name="direction";value=1},{name="long_crc";value=16;}
				/*	,{name="print_nof_pkts";value=10000} */
			); 
	};	
	
};


interfaces:
(
	{src="source";dest="crc"},
	{src="crc";dest="coder"},
	{src="coder";dest="modulator"},	
	{src="modulator";dest="demodulator"},	
	{src="demodulator";dest="decoder"},
	{src="decoder";dest="crc_check"}
);


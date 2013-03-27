modules:
{
	
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
	
	ratematching:
	{
		binary="modrep_osld/liblte_ctrl_ratematching.so";	
		mopts=8;
		variables=(
			{name="direction";value=0},{name="E";value=0}
		);
	};
	
	scrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=11;
		variables=({name="subframe";value=0},{name="cell_gr";value=2},{name="cell_sec";value=0},{name="channel";value=2},{name="direct";value=0});
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=11;
		variables=({name="modulation";value=2;});
	};
		
	
};


interfaces:
(
	{src="_input";dest="crc"},
	{src="crc";dest="coder"},
	{src="coder";dest="ratematching"},
	{src="ratematching";dest="scrambling"},	
	{src="scrambling";dest="modulator"},	
	{src="modulator";dest="_output"}
);


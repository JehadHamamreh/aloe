modules:
{
	
	pack:
	{
		binary="modrep_osld/liblte_bch_pack.so";	
		mopts=4;
		variables=(
			{name="direction";value=0},
			{name="enable";value=0},
			{name="nof_prb";value=6},{name="sfn";value=0}
		);
	};

	
	crc:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=3;
		variables=({name="long_crc";value=16;});
	};	
	
	crc_scramble:
	{
		binary="modrep_osld/liblte_crc_scrambling.so";
		mopts=2;
		variables=({name="direction";value=0;},{name="channel";value=1;},{name="nof_ports";value=1;});
	};	
	
	coder:
	{
		binary="modrep_osld/libgen_convcoder.so";	
		mopts=3;
		variables=(
			{name="constraint_length";value=7},{name="rate";value=3},{name="tail_bit";value=1},
			{name="generator_0";value=91},{name="generator_1";value=127},{name="generator_2";value=117}
		);
	};
		
	
	ratematching:
	{
		binary="modrep_osld/liblte_ctrl_ratematching.so";	
		mopts=3;
		variables=(
			{name="direction";value=0},{name="E";value=1920}
		);
	};
	
	demux:
	{
		binary="modrep_osld/liblte_bch_demux.so";	
		mopts=2;
	};

	scrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=3;
		variables=({name="subframe";value=0},{name="cell_gr";value=2},{name="cell_sec";value=0},
		{name="channel";value=3},{name="hard";value=1});
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=3;
		variables=({name="modulation";value=2;});
	};
		
	
};


interfaces:
(
	{src="_input";dest="pack"},
	{src="pack";dest="crc"},
	{src="crc";dest="crc_scramble"},
	{src="crc_scramble";dest="coder"},
	{src="coder";dest="ratematching"},
	{src="ratematching";dest="demux"},
	{src="demux";dest="scrambling"},
	{src="scrambling";dest="modulator"},
	{src="modulator";dest="_output"}
);


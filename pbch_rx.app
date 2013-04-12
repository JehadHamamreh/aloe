modules:
{
	
	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=50;
		variables=(
			{name="soft";value=1;},{name="modulation";value=2;},{name="sigma2";value=1.5;});
	};
	
	descrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=11;
		variables=({name="subframe";value=0},{name="q";value=0;},{name="cell_gr";value=2},{name="cell_sec";value=0},
			{name="channel";value=3},{name="hard";value=0});
	};
	
	unratematching:
	{
		binary="modrep_osld/liblte_ctrl_ratematching.so";	
		mopts=8;
		variables=(
			{name="direction";value=1},{name="S";value=120}
		);
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
	
	crc_descramble:
	{
		binary="modrep_osld/liblte_crc_scrambling.so";
		mopts=6;
		variables=({name="direction";value=0;},{name="channel";value=1;},{name="nof_ports";value=1;});
	};	

	crc_check:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=5;
		variables=({name="direction";value=1},{name="long_crc";value=16;}
					,{name="print_nof_pkts";value=1} 
			); 
	};	
	
	unpack:
	{
		binary="modrep_osld/liblte_bch_pack.so";	
		mopts=8;
		variables=(
			{name="direction";value=1}
		);
	};
	
};


interfaces:
(	
	{src="_input";dest="demodulator"},
	{src="demodulator";dest="descrambling"},
	{src="descrambling";dest="unratematching"},
	{src="unratematching";dest="decoder"},
	{src="decoder";dest="crc_descramble"},
	{src="crc_descramble";dest="crc_check"},	
	{src="crc_check";dest="unpack"},	
	{src="unpack";dest="_output"}	
);


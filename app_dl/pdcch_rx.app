modules:
{
	
	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=9;
		variables=(
			{name="soft";value=1;},{name="modulation";value=2;},{name="sigma2";value=1.5;});
	};
	
	descrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=6;
		log=false;
		variables=({name="subframe";value=-1},{name="q";value=0;},{name="cell_gr";value=2},{name="cell_sec";value=0},
		{name="hard";value=0},{name="channel";value=2});
	};

	unratematching:
	{
		binary="modrep_osld/liblte_ctrl_ratematching.so";	
		mopts=7;
		variables=(
			{name="direction";value=1},{name="S";value=0}
		);
	};
		
	decoder:
	{
		binary="modrep_osld/libgen_viterbi.so";	
		mopts=233;
		log=false;
		variables=(
			{name="constraint_length";value=7},{name="rate";value=3},
			{name="generator_0";value=91},{name="generator_1";value=127},{name="generator_2";value=117}
			);
	};

	crc_check:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=3;
		log=false;
		variables=({name="direction";value=1},
					{name="forward_on_error";value=1},
					{name="long_crc";value=16;}
				/*,{name="point_interval";value=0} */
				/*,{name="print_interval";value=1}  */
			); 
	};	
	
	unpack:
	{
		binary="modrep_osld/liblte_dci_pack.so";
		mopts=4;
		log=true;
		variables=({name="direction";value=1},{name="nof_rbg";value=6});
	};	
	
	
};


interfaces:
(
	{src="_input";dest="demodulator"},
	{src="demodulator";dest="descrambling"},
	{src="descrambling";dest="unratematching"},
	{src="unratematching";dest="decoder"},
	{src="decoder";dest="crc_check"},
	{src="crc_check";dest="unpack"},
	{src="unpack";dest="_output"}
);


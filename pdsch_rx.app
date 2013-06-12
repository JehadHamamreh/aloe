
modules:
{
	
	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=60;
		log=false;
		variables=(
			{name="soft";value=1;},{name="modulation";value=2;},{name="sigma2";value=0.5;});
	};
	
	descrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=26;
		log=false;
		variables=({name="subframe";value=-1},{name="q";value=0;},{name="cell_gr";value=2},{name="cell_sec";value=0},
		{name="hard";value=0},{name="channel";value=0});
	};
	
	unratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=122;
		log=true;
		variables=(
			{name="direction";value=1},{name="out_len";value=0},
			{name="rvidx";value=0;}
		);
	};
	
	decoder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=279;
		log=true;
		variables=({name="direction";value=1},{name="iterations";value=1},{name="padding";value=0;});
	};

	uncrc_tb:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=16;
		log=true;
		variables=({name="direction";value=1},{name="long_crc";value=16;}
					/*,{name="point_interval";value=10}*/
					,{name="print_interval";value=900}
					,{name="forward_on_error";value=1}
			); 
	};	
	
};


interfaces:
(
	{src="_input";dest="demodulator"},	
	{src="demodulator";dest="descrambling"},	
	
	{src="descrambling";dest="unratematching"},	
	{src="unratematching";dest="decoder"},	
	{src="decoder";dest="uncrc_tb"},	
	{src="uncrc_tb";dest="_output"}
);


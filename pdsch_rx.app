
modules:
{
	descrambling:
	{
		binary="modrep_osld/liblte_descrambling.so";	
		mopts=11;
		variables=({name="tslot_idx";value=0},{name="q";value=0;},{name="cell_gr";value=2},{name="cell_sec";value=0});
	};
	
	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=50;
		variables=(
			{name="soft";value=1;},{name="modulation";value=2;},{name="sigma2";value=0.1;});
	};
	
	unratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=65;
		variables=(
			{name="direction";value=1},{name="out_len";value=0},
			{name="rvidx";value=0;}
		);
	};
	
	decoder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=180;
		variables=({name="direction";value=1},{name="padding";value=0;});
	};

	uncrc_tb:
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
	{src="_input";dest="descrambling"},	
	{src="descrambling";dest="demodulator"},	
	
	{src="demodulator";dest="unratematching"},	
	{src="unratematching";dest="decoder"},	
	{src="decoder";dest="uncrc_tb"},	
	{src="uncrc_tb";dest="_output"}
);


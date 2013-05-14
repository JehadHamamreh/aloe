
modules:
{

	demodulator:
	{
		binary="modrep_osld/libgen_soft_demod.so";	
		mopts=75;
		variables=(
			{name="modulation";value=2});
	};	/* demodulation = 1 (BPSK), 2 (QPSK), 4 (16QAM), or 6 (64QAM) */
	
	descrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=35;
		variables=({name="subframe";value=0},{name="q";value=0;},{name="cell_gr";value=2},{name="cell_sec";value=0},
		{name="hard";value=0},{name="channel";value=0});
	};

	unratematching:
	{
		binary="modrep_osld/liblte_ratematching.so";	
		mopts=84;
		variables=(
			{name="direction";value=1},
			{name="out_len";value=3060},
			{name="rvidx";value=0;}
		);
	};

	decoder:
	{
		binary="modrep_osld/liblte_turbocode.so";	
		mopts=200;
		variables=({name="direction";value=1},{name="iterations";value=1},{name="padding";value=0;});
	};

	uncrc_tb:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=8;
		variables=({name="direction";value=1},{name="long_crc";value=16;},
			{name="point_interval";value=100},{name="print_interval";value=0}); 
	};	
	
};


interfaces:
(
	{src="_input"; dest="demodulator"},			
	{src="demodulator"; dest="descrambling"},
	{src="descrambling"; dest="unratematching"},
	{src="unratematching"; dest="decoder"},
	{src="decoder"; dest="uncrc_tb"},
	{src="uncrc_tb"; dest="_output"}
);


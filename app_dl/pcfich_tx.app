
modules:
{

	coder:
	{
		binary="modrep_osld/liblte_cfi_coding.so";	
		mopts=3;
		variables=(
			{name="cfi";value=-1}
		);
	};

	scrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=5;
		variables=(
			{name="cell_gr";value=101},{name="cell_sec";value=2},{name="channel";value=1},{name="hard";value=1},
			{name="direct";value=0}
		);
	};

	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=4;
		variables=(
			{name="modulation";value=2}
		);
	};
		
};


interfaces:
(
	{src="coder";dest="scrambling"},
	{src="scrambling";dest="modulator"},
	{src="modulator";dest="_output"}
);


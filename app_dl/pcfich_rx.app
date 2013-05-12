
modules:
{


	demodulation:
	{
		binary="modrep_osld/libgen_hard_demod.so";	
		mopts=4;
		variables=(
			{name="modulation";value=2}
		);
	};
	
	descrambling:
	{
		binary="modrep_osld/liblte_scrambling.so";	
		mopts=4;
		variables=(
			{name="cell_gr";value=101},{name="cell_sec";value=2},{name="channel";value=1},
			{name="direct";value=1},{name="hard";value=1}
		);
	};
		
	decoder:
	{
		binary="modrep_osld/liblte_cfi_decoding.so";	
		mopts=3;
	};
		
};


interfaces:
(
	{src="_input";dest="demodulation"},
	{src="demodulation";dest="descrambling"},
	{src="descrambling";dest="decoder"},
	{src="decoder";dest="_output"}
);


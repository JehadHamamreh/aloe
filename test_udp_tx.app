modules:
{
	
	source:
	{
		binary="modrep_default/libsource.so";	
		mopts=8;
		variables=(
			{name="block_length";value=15344},{name="generator";value=0}
		);
	};

	
	crc:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=6;
		variables=({name="long_crc";value=16;});
	};	
	
	sink:
	{
		binary="modrep_default/libudp_sink.so";
		mopts=5;
		variables=({name="address";value="192.168.0.1"},{name="port";value=10000;},{name="nof_pkts";value=15}
			); 
	};	
	
};


interfaces:
(
	{src="source";dest="crc"},
	{src="crc";dest="sink"}
);


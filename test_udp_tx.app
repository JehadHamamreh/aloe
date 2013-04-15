modules:
{
	
	source:
	{
		binary="modrep_default/libsource.so";	
		mopts=8;
		variables=(
			{name="block_length";value=24},{name="generator";value=0}
		);
	};

	
	crc:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=6;
		variables=({name="long_crc";value=16;});
	};	
	
	bit_pack:
	{
		binary="modrep_default/libbitpacker.so";	
		mopts=8;
		variables=({name="direction";value=1}); 
	};
	
	
	sink:
	{
		binary="modrep_default/libudp_sink.so";
		mopts=5;
		variables=({name="address";value="127.0.0.1"},{name="port";value=10000;}
			); 
	};	
	
};


interfaces:
(
	{src="source";dest="crc"},
	{src="crc";dest="bit_pack"},
	{src="bit_pack";dest="sink"}
);


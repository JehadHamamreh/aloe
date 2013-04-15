modules:
{
	
	source:
	{
		binary="modrep_default/libudp_source.so";	
		mopts=8;
		variables=({name="address";value="127.0.0.1"},{name="port";value=10000;}
			); 
	};
		
	bit_pack:
	{
		binary="modrep_default/libbitpacker.so";	
		mopts=8;
		variables=({name="direction";value=0}); 
	};
	
	
	bit_unpack:
	{
		binary="modrep_default/libbitpacker.so";	
		mopts=8;
		variables=({name="direction";value=1}); 
	};

	sink:
	{
		binary="modrep_default/libudp_sink.so";
		mopts=5;
		variables=({name="address";value="127.0.0.1"},{name="port";value=20000;}
			); 
	};	
	
};


interfaces:
(
	{src="source";dest="bit_pack"},
	{src="bit_pack";dest="bit_unpack"},
	{src="bit_unpack";dest="sink"}
);


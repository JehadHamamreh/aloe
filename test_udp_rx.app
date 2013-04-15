modules:
{
	
	source:
	{
		binary="modrep_default/libudp_source.so";	
		mopts=8;
		variables=({name="address";value="127.0.0.1"},{name="port";value=10000;}
			); 
	};
	
	bit_unpack:
	{
		binary="modrep_default/libbitpacker.so";	
		mopts=8;
		variables=({name="direction";value=0}); 
	};
	

	
	crc_check:
	{
		binary="modrep_osld/libgen_crc.so";
		mopts=5;
		variables=({name="direction";value=1},{name="long_crc";value=16;}
					,{name="print_nof_pkts";value=1} 
			); 
	};	
	
};


interfaces:
(
	{src="source";dest="bit_unpack"},
	{src="bit_unpack";dest="crc_check"}
);


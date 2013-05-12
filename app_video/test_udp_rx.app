modules:
{
	
	source:
	{
		binary="modrep_default/libudp_source.so";	
		mopts=8;
		variables=({name="address";value="0.0.0.0"},{name="port";value=10000;}
				,{name="block_length";value=15360} 
			); 
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
	{src="source";dest="crc_check"}
);


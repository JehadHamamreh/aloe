modules:
{
	
	source:
	{
		binary="modrep_default/libdac_source.so";	
		mopts=8;
	};
	
	
	sink:
	{
		binary="modrep_default/libfile_sink.so";
		mopts=5;
		variables=({name="file_name";value="dac_capture.bin"}); 
	};	
	
};


interfaces:
(
	{src="source";dest="sink";}
);

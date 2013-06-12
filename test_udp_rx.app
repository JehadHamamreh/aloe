
modules:
{
	
	source:
	{
		binary="modrep_default/libdac_source.so";	
		mopts=8;
	};
	
	
	file:
	{
		binary="modrep_default/libfile_sink.so";
		mopts=5;
		variables=({name="file_name";value="output.bin"});
	};	
	
};


interfaces:
(
	{src="source";dest="file"}
);


modes: 
(
	{name="fft128qpsk"; desc="PDSCH-TX 128 QPSK";},
	{name="fft128qam16";desc="PDSCH-TX 128 QAM16";},
	{name="fft128qam64";desc="PDSCH-TX 128 QAM64";},

	{name="fft256qpsk"; desc="PDSCH-TX 256 QPSK";},
	{name="fft256qam16";desc="PDSCH-TX 256 QAM16";},
	{name="fft256qam64";desc="PDSCH-TX 256 QAM64";},

	{name="fft512qpsk"; desc="PDSCH-TX 512 QPSK";},
	{name="fft512qam16"; desc="PDSCH-TX 512 QAM16";},
	{name="fft512qam64"; desc="PDSCH-TX 512 QAM64";},

	{name="fft1024qpsk"; desc="PDSCH-TX 1024 QPSK";},
	{name="fft1024qam16"; desc="PDSCH-TX 1024 QAM16";},
	{name="fft1024qam64"; desc="PDSCH-TX 1024 QAM64";},

	{name="fft2048qpsk"; desc="PDSCH-TX 2048 QPSK";},
	{name="fft2048qam16"; desc="PDSCH-TX 2048 QAM16";},
	{name="fft2048qam64"; desc="PDSCH-TX 2048 QAM64";}

);

modules:
{
	source:
	{
		binary="modrep_default/libsource.so";	
		mopts=23;
		variables=(
			{name="block_length";value=(
				("fft128qpsk",2016),
				("fft128qam16",1000),
				("fft128qam64",3000),

				("fft256qpsk",1000),
				("fft256qam16",2000),
				("fft256qam64",3000),
				
				("fft512qpsk",2000),
				("fft512qam16",4000),
				("fft512qam64",6000),
	
				("fft1024qpsk",4000),
				("fft1024qam16",8000),
				("fft1024qam64",12000),

				("fft2048qpsk",8000),
				("fft2048qam16",16000),
				("fft2048qam64",24000)
			);},
			{name="generator";value=0;}
		);
	};


	modulator:
	{
		binary="modrep_osld/libgen_modulator.so";	
		mopts=30;
		variables=({name="modulation";value=(
				("fft128qpsk",1),
				("fft128qam16",2),
				("fft128qam64",3),

				("fft256qpsk",1),
				("fft256qam16",2),
				("fft256qam64",3),

				("fft512qpsk",1),
				("fft512qam16",2),
				("fft512qam64",3),

				("fft1024qpsk",1),
				("fft1024qam16",2),
				("fft1024qam64",3),

				("fft2048qpsk",1),
				("fft2048qam16",2),
				("fft2048qam64",3)
			);});
	};

	padding:
	{
		binary="modrep_osld/libgen_padding.so";	
		mopts=10;
		variables=(
			{name="data_type";value=0;},
			{name="pre_padding";value=(
				("fft128qpsk",28),
				("fft128qam16",28),
				("fft128qam64",28),

				("fft256qpsk",38),
				("fft256qam16",38),
				("fft256qam64",38),

				("fft512qpsk",106),
				("fft512qam16",106),
				("fft512qam64",106),

				("fft1024qpsk",212),
				("fft1024qam16",212),
				("fft1024qam64",212),

				("fft2048qpsk",424),
				("fft2048qam16",424),
				("fft2048qam64",424)
			);},
			{name="post_padding";value=(
				("fft128qpsk",28),
				("fft128qam16",28),
				("fft128qam64",28),

				("fft256qpsk",38),
				("fft256qam16",38),
				("fft256qam64",38),

				("fft512qpsk",106),
				("fft512qam16",106),
				("fft512qam64",106),

				("fft1024qpsk",212),
				("fft1024qam16",212),
				("fft1024qam64",212),

				("fft2048qpsk",424),
				("fft2048qam16",424),
				("fft2048qam64",424)

			);},
			{name="nof_packets";value=14;}
		);
	};

	ifft:
	{
		binary="modrep_osld/libgen_dft.so";
		mopts=84;
		stage=9;
		variables=(
			{name="direction";value=0;},
			{name="mirror";value=1;},
			{name="normalize";value=1;},
			{name="dft_size";value=(
				("fft128qpsk",128),
				("fft128qam16",128),
				("fft128qam64",128),

				("fft256qpsk",256),
				("fft256qam16",256),
				("fft256qam64",256),

				("fft512qpsk",512),
				("fft512qam16",512),
				("fft512qam64",512),

				("fft1024qpsk",1024),
				("fft1024qam16",1024),
				("fft1024qam64",1024),

				("fft2048qpsk",2048),
				("fft2048qam16",2048),
				("fft2048qam64",2048)
			);}
		);
	};

	sink:
	{
		binary="modrep_default/libdac_sink.so";
		mopts=5;
		variables=(
			{name="gain";value=0.006},
			{name="mode";value=3},
			{name="is_complex";value=1},
			{name="freq_samp";value=(
				("fft128qpsk",1920000.0),
				("fft128qam16",1920000.0),
				("fft128qam64",1920000.0),

				("fft256qpsk",3840000.0),
				("fft256qam16",3840000.0),
				("fft256qam64",3840000.0),				

				("fft512qpsk",7680000.0),
				("fft512qam16",7680000.0),
				("fft512qam64",7680000.0),

				("fft1024qpsk",15360000.0),
				("fft1024qam16",15360000.0),
				("fft1024qam64",15360000.0),

				("fft2048qpsk",30720000.0),
				("fft2048qam16",30720000.0),
				("fft2048qam64",30720000.0)
			);}
		);		
	};
};

interfaces:
(
	{src="source";dest="modulator"},
	{src="modulator";dest="padding"},	
	{src="padding";dest="ifft"},	
	{src="ifft";dest="sink"}
);

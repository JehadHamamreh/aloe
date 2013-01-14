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
		stage=1;
		variables=(
			{name="block_length";value=(
				("fft128qpsk",500),
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

	crc_tb:
	{
		binary="modrep_ofdm/libgen_crc.so";
		mopts=17.8;
		stage=2;
		variables=({name="long_crc";value=24;});
	};	

/* codeblock segmentation */
	segmentation:
	{
		binary="modrep_ofdm/liblte_segmentation.so";
		mopts=3;
		stage=3;
	};	

/* codeblock coding */

/* there are multiple branches of these two modules */

	crc_cb:
	{
		binary="modrep_ofdm/libgen_crc.so";
		mopts=6;
		instances=3;
		stage=4;
		variables=({name="long_crc";value=24;});
	};	

	coder:
	{
		binary="modrep_ofdm/liblte_encoder.so";	
		mopts=6;
		instances=3;
		stage=5;
		variables=({name="padding";value=0;});
	};
	
/* end of codeblock coding */

	ratematching:
	{
		binary="modrep_ofdm/liblte_ratematching.so";	
		mopts=40;
		stage=6;
		variables=(
			{name="out_len";value=(
				("fft128qpsk",1008),
				("fft128qam16",2016),
				("fft128qam64",3024),

				("fft256qpsk",2520),
				("fft256qam16",5040),
				("fft256qam64",7560),
				
				("fft512qpsk",4200),
				("fft512qam16",8400),
				("fft512qam64",12600),

				("fft1024qpsk",8400),
				("fft1024qam16",16800),
				("fft1024qam64",25200),

				("fft2048qpsk",16800),
				("fft2048qam16",33600),
				("fft2048qam64",50400)
			);},
			{name="rvidx";value=0;}
		);
	};

	modulator:
	{
		binary="modrep_ofdm/libgen_modulator.so";	
		mopts=30;
		stage=7;
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
		binary="modrep_ofdm/libgen_padding.so";	
		mopts=10;
		stage=8;
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
			{name="nof_packets";value=7;}
		);
	};

	ifft:
	{
		binary="modrep_ofdm/libgen_dft.so";
		mopts=84;
		stage=9;
		variables=(
			{name="direction";value=1;},
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

	cyclic:
	{
		binary="modrep_ofdm/libgen_cyclic.so";	
		mopts=28;
		stage=10;
		variables=(
			{name="ofdm_symbol_sz";value=(
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

			);},
			{name="cyclic_prefix_sz";value=(
				("fft128qpsk",9),
				("fft128qam16",9),
				("fft128qam64",9),
				
				("fft256qpsk",18),
				("fft256qam16",18),
				("fft256qam64",18),
				
				("fft512qpsk",36),
				("fft512qam16",36),
				("fft512qam64",36),

				("fft1024qpsk",72),
				("fft1024qam16",72),
				("fft1024qam64",72),

				("fft2048qpsk",144),
				("fft2048qam16",144),
				("fft2048qam64",144)
			);},
			{name="first_cyclic_prefix_sz";value=(
				("fft128qpsk",10),
				("fft128qam16",10),
				("fft128qam64",10),

				("fft256qpsk",20),
				("fft256qam16",20),
				("fft256qam64",20),				
				
				("fft512qpsk",40),
				("fft512qam16",40),
				("fft512qam64",40),

				("fft1024qpsk",80),
				("fft1024qam16",80),
				("fft1024qam64",80),

				("fft2048qpsk",160),
				("fft2048qam16",160),
				("fft2048qam64",160)
			);}
		);
	};

	sink:
	{
		binary="modrep_default/libplp_sink.so";
		mopts=5;
		stage=11;
		variables=(
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
	{src=("source",0);dest=("crc_tb",0)},
	{src="crc_tb";dest="segmentation"},
	
	{src=("segmentation",0);dest="crc_cb_0";},	
	{src="crc_cb_0";dest="coder_0";},
	{src="coder_0";dest=("ratematching",0);},	

	{src=("segmentation",1);dest="crc_cb_1";},	
	{src="crc_cb_1";dest="coder_1";},
	{src="coder_1";dest=("ratematching",1);},	

	{src=("segmentation",2);dest="crc_cb_2";},	
	{src="crc_cb_2";dest="coder_2";},
	{src="coder_2";dest=("ratematching",2);},	
/*
	{src=("segmentation",3);dest="crc_cb_3";},	
	{src="crc_cb_3";dest="coder_3";},
	{src="coder_3";dest=("ratematching",3);},	

	{src=("segmentation",4);dest="crc_cb_4";},	
	{src="crc_cb_4";dest="coder_4";},
	{src="coder_4";dest=("ratematching",4);},	

	{src=("segmentation",5);dest="crc_cb_5";},	
	{src="crc_cb_5";dest="coder_5";},
	{src="coder_5";dest=("ratematching",5);},	
*/
	{src="ratematching";dest="modulator"},	
	{src="modulator";dest="padding"},	
	{src="padding";dest="ifft"},	
	{src="ifft";dest="cyclic"},	
	{src="cyclic";dest="sink"}	
);

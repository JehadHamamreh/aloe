

addpath('/usr/local/mex');  % mex files: CRC, etc.

% test crc
D = 24;
input = (randi(2,1,D)==1);
crc_out = am_gen_crc(input)
crc_out2 = am_gen_crc(input, {{'direction',int32(0)}, {'long_crc',int32(16)}, {'poly',int32(11021)}})


uncrc_out = am_gen_crc(crc_out, {{'direction',int32(1)}})
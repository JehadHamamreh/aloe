
clear
N=100;

x=randi(2,N,1)==1;

crcb=am_gen_crc(x,{{'direction',int32(0)}});
uncrc=am_gen_crc(crcb,{{'direction',int32(1)}});

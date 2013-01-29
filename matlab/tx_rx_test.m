clear
addpath('/usr/local/mex')
N=500;
noise=0;

crc_params={{'long_crc',int32(24)}};
uncrc_params={{'long_crc',int32(24)},{'mode',int32(1)}};
coder_params={{'padding',int32(0)}};
rm_params={{'out_len',int32(1008)},{'rvidx',int32(0)},{'data_type',int32(0)}};
unrm_params={{'out_len',int32(3*N+12)},{'rvidx',int32(0)},{'data_type',int32(1)}};
modulator_params={{'modulation',int32(1)}};

src = randi(2,N,1)==1;

%TX
enc=am_lte_encoder(src,coder_params);
rm=am_lte_ratematching(enc,rm_params);
mod=am_gen_modulator(rm,modulator_params);

% channel
mod=mod+0.7*randn(size(mod));

%RX 
soft=soft_demapper(mod, 1, 2, 10^-3);
unrm=am_lte_ratematching(soft,unrm_params);
out=am_lte_decoder(unrm,coder_params);
errors=sum(src~=out')






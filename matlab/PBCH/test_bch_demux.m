% Test lte_bch_demux

addpath('/home/vuk/DATOS/workspace/lte_bch_demux');         % LTE BCH Time Demultiplexing


for i=0:39
    if (mod(i,40)==0)
        input = randi(2,1,20)==1
    else
        input = {};
    end
    
    in_demux_l(i+1) = length(input)
    demux_outx = am_lte_bch_demux(input)
    out_demux_l(i+1) = length(demux_outx)
    
end

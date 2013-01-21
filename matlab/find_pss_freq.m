function find_pss(input)

chu0=lte_pss_zc(0);
chu0=[chu0(1:31);0;chu0(32:end)];
y=[];
for i=1:length(input)/128
    signal=input((i-1)*128+1:i*128);
    y=[y xcorr([signal(128-30:128) signal(1:32)],chu0)];
end
plot(abs(y))
[val pos]=max(y)

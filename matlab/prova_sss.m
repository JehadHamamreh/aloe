addpath('/home/ismael/aloe_ws/eclipse_aloe/lib/modrep_osld');

Nid_1=0;
Nid_2=0;

[s0 s5 c0 c1 m0 m1] = lte_generate_sss(Nid_1,Nid_2);

%y=(s0+rand(size(s0))).*exp(-(1:length(s0)).*1i*pi/2);

x=reshape(xf(:,16:17),[],1);

subplot(1,2,1)

fs=find_pss(x,0);

xfs=transpose(x(fs:(fs+1920-1)));
y=xfs;

y=y((960-2*137+1):(960-137-9));
yf=fft(y,128);
y=[yf(98:128) yf(2:32)];

[m0 m1 x zprod0 zprod1]=find_sss(xfs,0,c0,c1);

x2=transpose(am_lte_sss_synch(xfs,{}));

%plot(1:62,real(x2),1:62,real(y))

subplot(1,2,2)
plot(1:62,abs(x2),1:62,abs(x))
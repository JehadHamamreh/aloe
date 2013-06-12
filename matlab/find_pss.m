function [ fs eps] = find_pss( x, N_id_2, doplot)
if nargin==2
    doplot=false;
end
    c=lte_pss_zc(N_id_2);
    cc=[zeros(33,1); c; zeros(33,1)];
    ccf=[0; cc(65:128); cc(2:64)];
    ccf=conj([ifft(ccf)]);
    
    w2=conv(x,ccf);
    if (doplot)
        plot(abs(w2))
    end
    [m i]=max(w2);
    fs=i-960;
    if (doplot)
        fprintf('Frame start at %d\n',fs);
    end
    
%    y=ccf.*x(i-128:i-1);
%    yy=w2(i-128:i-1);
%    y0=y(1:64);
%    y1=y(65:length(y));    
%    eps=angle(conj(sum(y0))*sum(y1))/pi;
    eps=0;
    if (doplot)
        fprintf('epsilon=%g\n',eps);
    end
end


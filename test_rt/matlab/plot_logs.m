function tscR=plot_logs(npipes)
    if nargin==0
        npipes=1;
    end
    for i=1:npipes
        p{i}=read_period_ns(sprintf('./timer_in_%d.log',i-1))/1000;
        tin=read_long(sprintf('./timer_in_%d.log',i-1));
        tou=read_long(sprintf('./timer_out_%d.log',i-1));
        m=min(length(tou),length(tin));
        te=tou(1:m)-tin(1:m);
        te(te<0)=te(te<0)+10^9;
        tex{i}=te/1000;
        tsc{i}=read_longlong(sprintf('./work_%d.log',i-1));
    end

    for i=1:npipes
       x=tsc{i};
       y=reshape(x,100,[]);
       n=size(y);
       tsce=zeros(n(2),1);
       d=zeros(90,n(2));
       for j=1:n(2)
           y(:,j)=y(:,j)-y(1,j);
           d(:,j)=diff(y(10:100,j));
           tsce(j)=y(100,j);
       end
       tscR{i}=d;
       tscdiff{i}=reshape(d,1,[]);
    end
    
    for i=1:npipes
        subplot(npipes,3,(i-1)*3+1)
        plot(tex{i})
        xlabel('Time slot')
        ylabel('Exec time [\mus]')
        subplot(npipes,3,(i-1)*3+2)
        plot(p{i})
        xlabel('Time slot')
        ylabel('Period [\mus]')
        subplot(npipes,3,(i-1)*3+3)
        plot(tscdiff{i})
        xlabel('Time')
        ylabel('TSC')
    end

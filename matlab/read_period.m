function [period]=read_period(filename,count)

if (nargin==1)
    count=inf;
end
ti=read_file(filename,count);
n=length(ti);
period=ti(2:n)-ti(1:(n-1));

period(period<0)=10^6+period(period<0);
period(1)=mean(period(2:(n-1)));




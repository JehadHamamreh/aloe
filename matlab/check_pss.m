function [ fs ] = check_pss( x )
%CHECK_PSS Summary of this function goes here
%   Detailed explanation goes here
flen=9600;
n=length(x);
nf=floor(n/flen);

xf=reshape(x(1:nf*flen),flen,[]);

fs=zeros(nf,1);
for i=1:nf
    fs(i)=find_pss(xf(:,i),0);
end

plot(fs)

end



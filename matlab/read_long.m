function [out]=read_long(filename,count)

[tidin msg]=fopen(filename,'r');
if (tidin==-1)
    fprintf('error opening %s: %s\n',filename, msg);
    out=[];
    return
end

if (nargin==1) 
    count=inf;
end

out=fread(tidin,count,'long');
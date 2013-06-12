clear;
npipes=2;
TH=1;

timer=read_period('./timer.time');
for i=1:npipes
    p{i}=read_period(sprintf('./pipe_in_%d.time',i-1));
    k{i}=read_file(sprintf('./exec_%d.time',i-1));
    p{i}=p{i}(TH:length(p{i}));
    k{i}=k{i}(TH:length(k{i}));
    tin=read_file(sprintf('./pipe_in_%d.time',i-1));
    tou=read_file(sprintf('./pipe_out_%d.time',i-1));
    m=min(length(tou),length(tin));
    te=tou(1:m)-tin(1:m);
    te(te<0)=te(te<0)+10^6;
    tex{i}=te(TH:length(te));
end

for i=1:npipes
subplot(npipes,2,(i-1)*2+1)
plot(tex{i})
xlabel('Time slot')
ylabel(strcat(sprintf('Exec time %d',i),' [\mus]'))
subplot(npipes,2,(i-1)*2+2)
plot(p{i})
xlabel('Time slot')
ylabel(strcat(sprintf('Period %d',i),' [\mus]'))
%subplot(npipes,3,(i-1)*3+3)
%plot(k{i})
end

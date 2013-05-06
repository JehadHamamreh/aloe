clear;
npipes=1;
TH=1;

timer=read_period('../reports/timer.time');
for i=1:npipes
    p{i}=read_period(sprintf('../reports/pipe_in_%d.time',i-1));
    k{i}=read_file(sprintf('../reports/exec_%d.time',i-1));
    p{i}=p{i}(TH:length(p{i}));
    k{i}=k{i}(TH:length(k{i}));
    tin=read_file(sprintf('../reports/pipe_in_%d.time',i-1));
    tou=read_file(sprintf('../reports/pipe_out_%d.time',i-1));
    te=tou-tin;
    te(te<0)=te(te<0)+10^6;
    tex{i}=te(TH:length(te));
end

for i=1:npipes
    subplot(npipes,3,(i-1)*3+1)
    plot(tex{i})
    subplot(npipes,3,(i-1)*3+2)
    plot(p{i})
    subplot(npipes,3,(i-1)*3+3)
    plot(k{i})
end

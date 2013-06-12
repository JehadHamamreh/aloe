files = dir('./');

fileIndex = find(~[files.isdir]);

skip={'exec_0.time','exec_1.time','pipe_in_0.time','pipe_in_1.time','pipe_out_0.time','pipe_out_1.time','timer.time'};

leg=[];
x=[];
t=1;
for i = 1:length(fileIndex)

fileName = files(fileIndex(i)).name;
% Do stuff

    exclude=false;
    for j=1:length(skip) 
        if (strcmp(fileName,skip{j}))
            exclude=true;
        end
    end
    if (~exclude && length(findstr(fileName,'.time'))>0)        
        x{t}=read_file(fileName);
        leg=strvcat(leg,fileName);
        t=t+1;
    end
end
s=length(x);
y=[];
leg2=[];
for i=1:s
    idx=find(x{i}>50);
   if (length(idx)>0)
       plot(x{i})
       hold on
       leg2=strvcat(leg2,leg(i,:));
   end
end
hold off
legend(leg2)

    

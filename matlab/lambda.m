function [ y] = lambda( r, L , N)
    
y=zeros(1,length(r)-2*N);
for theta=1:length(y)
    for i=theta:theta+L-1
        y(theta)=y(theta)+r(i)*conj(r(i+N));
    end
end

end


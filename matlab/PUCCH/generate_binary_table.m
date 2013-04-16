

function x = generate_binary_table(bits)

x = zeros(2^bits,bits);

for i=0:2^bits-1
    int_value = i;
    for j=(bits-1):-1:0
        if (2^j <= int_value)
            x(i+1,bits-j) = 1;
            int_value = int_value - 2^j;
        else
            x(i+1,bits-j) = 0;
        end
    end
end

end
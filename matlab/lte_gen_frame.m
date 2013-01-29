function [ output ] = lte_gen_frame( params )
    
    output=false(1,sum(params.src_bits_x_slot(1:params.nof_slots)));
    k=1;
    for i=1:params.nof_slots
        l=params.src_bits_x_slot(i);
        output(k:(k+l-1))=randi(2,l,1)==1;
        k=k+l;
    end

end


clear
addpath('/usr/local/mex');

M=1000;
snr_db=linspace(-2,2,10);

lte_params=lte_readparams(2);

for s=1:length(snr_db)    
    errors=0;
    for i=1:M

        lte_params.sigma2=10^(-snr_db(s)/10);

        data_in = lte_gen_frame(lte_params);

        tx_frame = lte_tx_frame(data_in,lte_params);

        rx_frame = lte_channel(tx_frame,lte_params);

        data_out = lte_rx_frame(rx_frame,lte_params);
        
        err=sum(data_in~=data_out);
        errors=errors+err;
        if (err>0) 
            fprintf('%d errors!\n',err)
        end
        fprintf('--frame %d/%d (snr=%g)\n',i,M,snr_db(s));
    end
    ber(s)=errors/(M*length(data_in));
    total_errors(s)=errors;
    fprintf('snr=%g, errors=%d\n',snr_db(s),errors);
end
semilogy(snr_db,ber)

function [ params ] = lte_readparams( lteslots_x_timeslot )

    params.fft_size=128;
    params.Ntot=72;
    params.modulation=1; % qpsk
    params.bits_x_symb=2;
    params.nrb=4;
    params.mcs=0;
    params.nof_slots=20/lteslots_x_timeslot;
    params.src_bits_x_slot=120*ones(1,params.nof_slots);
    params.nof_fft_x_slot=7*lteslots_x_timeslot;
    params.cp=repmat([10 9*ones(1,6)],1,lteslots_x_timeslot);
    params.lteslots_x_timeslot = lteslots_x_timeslot;
    params.nof_samples_x_slot=params.nof_fft_x_slot*params.fft_size+sum(params.cp);
    params.sigma2=0.4;
    
end

clear
addpath('/usr/local/mex')
symb_x_slot=[132 204 276 480 276 480 276 480 276 480 132 480 276 480 276 480 276 480 276 480]; 

s=1;
sv=s:(s+1);
in = linspace(-4,4,sum(symb_x_slot(sv)))+1i*linspace(-4,4,sum(symb_x_slot(sv)));
rm = am_lte_resource_mapper(in,{{'direction',int32(0)},{'tslot_idx',int32(s-1)},{'lteslots_x_timeslot',int32(length(sv))}});
[out refsig] = am_lte_resource_mapper(rm,{{'direction',int32(1)},{'tslot_idx',int32(s-1)},{'lteslots_x_timeslot',int32(length(sv))}});

subplot(2,2,1)
plot(abs(in-out))
subplot(2,2,2)
plot(real(in))
subplot(2,2,3)
plot(real(out))
subplot(2,2,4)
plot(real(rm))

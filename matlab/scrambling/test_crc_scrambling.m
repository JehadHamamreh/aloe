

% test repetition coding & decoding

addpath('../common functions');

D = 30;
input = (randi(2,1,D)==1);
antenna_selection = 0;
dci_format = 0;
rnti = 4327;
ue_port=1;

out = crc_scrambling(input, antenna_selection, dci_format, rnti, ue_port, 1, 0)

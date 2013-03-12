#include <params.h>


/** Set to non-zero to send all remote parameters values each time slot automatically.
 * Otherwise, the user shall use ctrl_skeleton_send_idx() or ctrl_skeleton_send_name()
 * to manually send the parameter values
 */
int ctrl_send_always=1;

/* these are the parameters that will be changed in the rest of the modules by the ctrl module*/
struct remote_parameters {
	int tbs;
	int cbs;
	int modulation;
	int long_crc;
	int bits_x_slot;
	int tslot_idx;
	int cfi;
};

struct remote_parameters remote_params;

/* these are the parameters that can be changed in the ctrl module */
struct local_parameters {
	int mcs;
	int nrb;
	int fft_size;
	int cp_is_long;
};

struct local_parameters local_params;

remote_params_db_t remote_params_db[] = {

		{"source","block_length",&remote_params.tbs,sizeof(int)},

		{"pcfich_tx_coder","cfi",&remote_params.cfi,sizeof(int)},
		{"pcfich_tx_scrambling","subframe",&remote_params.tslot_idx,sizeof(int)},
		{"pcfich_rx_descrambling","subframe",&remote_params.tslot_idx,sizeof(int)},

		{"pdsch_tx_ratematching","out_len",&remote_params.bits_x_slot,sizeof(int)},
		{"pdsch_tx_scrambling","tslot_idx",&remote_params.tslot_idx,sizeof(int)},
		{"pdsch_tx_modulator","modulation",&remote_params.modulation,sizeof(int)},
		{"pdsch_rx_demodulator","modulation",&remote_params.modulation,sizeof(int)},
		{"pdsch_rx_descrambling","tslot_idx",&remote_params.tslot_idx,sizeof(int)},

		{"resmapp","subframe_idx",&remote_params.tslot_idx,sizeof(int)},
		{"resdemapp","subframe_idx",&remote_params.tslot_idx,sizeof(int)},

		{"pdsch_rx_unratematching","out_len",&remote_params.cbs,sizeof(int)},

		{NULL,NULL,NULL,0}}; /* etc */


my_params_db_t local_params_db[] = {
	{"mcs",&local_params.mcs,sizeof(int)},
	{"nrb",&local_params.nrb,sizeof(int)},
	{"fft_size",&local_params.fft_size,sizeof(int)},
	{"cp_is_long",&local_params.cp_is_long,sizeof(int)},
	{NULL,NULL,0}
};


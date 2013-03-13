

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
	int nof_pdsch;
	int pdsch_mask[2];
};

struct remote_parameters tx_params;
struct remote_parameters rx_params;

/* these are the parameters that can be changed in the ctrl module */
struct local_parameters {
	int mcs;
	int nrb;
	int fft_size;
	int cp_is_long;
};

struct local_parameters local_params;

remote_params_db_t remote_params_db[] = {

		{"source","block_length",&tx_params.tbs,sizeof(int)},

		{"pcfich_tx_coder","cfi",&tx_params.cfi,sizeof(int)},
		{"pcfich_tx_scrambling","subframe",&tx_params.tslot_idx,sizeof(int)},

		{"pdsch_tx_ratematching","out_len",&tx_params.bits_x_slot,sizeof(int)},
		{"pdsch_tx_scrambling","tslot_idx",&tx_params.tslot_idx,sizeof(int)},
		{"pdsch_tx_modulator","modulation",&tx_params.modulation,sizeof(int)},

		{"resmapp","subframe_idx",&tx_params.tslot_idx,sizeof(int)},

		/*		{"resmapp","cfi",&remote_params.cfi,sizeof(int)},
				{"resmapp","nof_pdsch",&remote_params.nof_pdsch,sizeof(int)},
				{"resmapp","pdsch_rbgmask_0",&remote_params.pdsch_mask[0],sizeof(int)},
		*/

		{"pcfich_rx_descrambling","subframe",&tx_params.tslot_idx,sizeof(int)},


		{"pdsch_rx_demodulator","modulation",&rx_params.modulation,sizeof(int)},
		{"pdsch_rx_descrambling","tslot_idx",&rx_params.tslot_idx,sizeof(int)},

		{"resdemapp_pcfich","subframe_idx",&rx_params.tslot_idx,sizeof(int)},

		{"resdemapp_pdsch","subframe_idx",&rx_params.tslot_idx,sizeof(int)},
		{"resdemapp_pdsch","cfi",&rx_params.cfi,sizeof(int)},

		{"resdemapp_pdsch","nof_pdsch",&rx_params.nof_pdsch,sizeof(int)},
		{"resdemapp_pdsch","pdsch_rbgmask_0",&rx_params.pdsch_mask[0],sizeof(int)},

		{"pdsch_rx_unratematching","out_len",&tx_params.cbs,sizeof(int)},

		{NULL,NULL,NULL,0}}; /* etc */


my_params_db_t local_params_db[] = {
	{"mcs",&local_params.mcs,sizeof(int)},
	{"nrb",&local_params.nrb,sizeof(int)},
	{NULL,NULL,0}
};


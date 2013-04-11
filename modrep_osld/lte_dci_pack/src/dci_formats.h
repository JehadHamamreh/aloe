#define MAX_RBG_SET	25

enum ra_type {
	NA,TYPE0, TYPE1
};

struct dci_format1 {
	char carrier_indicator;
	int carrier_indicator_len;
	enum ra_type ra_type;
	int rbg_mask[MAX_RBG_SET];
	int nof_rbg;
	int mcs;
	int harq_pnum;
	int harq_pnum_len;
	int new_data;
	int redundancy_version;
	int tpc_command;
};

int dci_format1_pack(char *buffer, struct dci_format1 *data);
int dci_format1_unpack(char *packet, int packet_len, struct dci_format1 *data);

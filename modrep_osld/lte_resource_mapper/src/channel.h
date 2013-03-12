
struct channel {
	const char *name;
	int type;
	int in_port;
	int out_port;
};

void read_channels(int *max_in_port, int *max_out_port);
int init_refsinc_signals();
int extract_refsig(void *input, void **out);
int check_received_samples_mapper();
int check_received_samples_demapper();
int allocate_all_channels(void **inp,void *output);
int deallocate_all_channels(void *input, void **out);

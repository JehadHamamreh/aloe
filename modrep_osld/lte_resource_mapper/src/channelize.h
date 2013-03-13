
struct channel {
	const char *name;
	int type;
	int in_port;
};

int read_channels();
int init_refsinc_signals();
int check_received_samples_mapper();
int allocate_all_channels(void **inp,void *output);

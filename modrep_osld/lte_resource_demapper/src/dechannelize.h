
struct channel {
	const char *name;
	int type;
	int param_value;
	int out_port;
};

int read_channels();
int copy_signal(void *in, void **out);
int extract_refsig(void *input, void **out);
int check_received_samples_demapper();
int channels_init_grid(int *channel_ids, int nof_channels);
int deallocate_all_channels(int *channel_ids, int nof_channels, void *input, void **out);

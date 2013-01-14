

#ifdef __cplusplus

extern "C" {
struct main_conf {
	char clock[16];
	char address[64];
	int chain_is_tx;
};
}


extern "C" int uhd_readcfg(struct dac_cfg *dac_cfg);

extern "C" int uhd_init(struct dac_cfg *cfg, long int *timeSlotLength, void (*sync)(void));

extern "C" void uhd_close();

extern "C" void uhd_setcfg(struct main_conf *main, struct dac_cfg *dac_cfg);


#else

struct main_conf {
	char clock[16];
	char address[64];
	int chain_is_tx;
};


int uhd_readcfg(struct dac_cfg *dac_cfg);

int uhd_init(struct dac_cfg *cfg, long int *timeSlotLength, void (*sync)(void));

void uhd_close();

void uhd_setcfg(struct main_conf *main, struct dac_cfg *dac_cfg);
#endif

#include <params.h>
#include <skeleton.h>

typedef struct {
	int module_idx;
	int variable_idx;
} va_t;

struct variable {
	const char *module_name;
	const char *variable_name;
};

#ifdef _DEFINE_VARIABLES_
const struct variable variables[] = {
			{"source","block_length"},
			{"crc_tb","crc_length"},
			{"ratematching","out_rm"}
			}; /* etc */
#endif

/* keep the same order */
#define TBSIZE 			1
#define CRCSIZE 		2
#define BITSXSLOT		3
#define NOF_VARIABLES	4


/* these are the parameters that can be changed in the ctrl module */
struct my_parameters {
	int mcs;
	int nrb;
	int fft_size;
};

struct my_params_def {
	const char *name;
	pmid_t pmid;
	void *value;
	int size;
};

#ifdef _DEFINE_VARIABLES_
struct my_parameters cur_pm;
struct my_params_def myparams[] = {
	{"mcs",NULL,&cur_pm.mcs,sizeof(int)},
	{"nrb",NULL,&cur_pm.nrb,sizeof(int)},
	{"fft_size",NULL,&cur_pm.fft_size,sizeof(int)},
	{NULL,NULL,NULL,0}
};
#endif


int ctrl_params_changed(int tslot, struct my_parameters *cur_pm);
int ctrl_params_tslot(int tslot, struct my_parameters *cur_pm);


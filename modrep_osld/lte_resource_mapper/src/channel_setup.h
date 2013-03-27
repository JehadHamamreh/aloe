#define ENABLE_PSS
#define ENABLE_SSS

#define ENABLE_REF

#define CHECK_RCV_SAMPLES

#define EXTRACT_REF_PORT	-1

#define CH_OFF -1

struct channel pdsch[] = {
		{"PDSCH_0",	CH_PDSCH, 0},
		{"PDSCH_1",	CH_PDSCH, -4},
		{NULL,-1, -1}
};

struct channel pdcch[] = {
		{"PDCCH_0", CH_PDCCH, 2},
		{"PDCCH_1", CH_PDCCH, -6},
		{NULL,-1, -1}
};

struct channel other[] = {
		{"PCFICH", CH_PCFICH, 1},
		{"PHICH", CH_PHICH, -1},
		{"PBCH", CH_PBCH, -10},
		{NULL,-1, -1}
};


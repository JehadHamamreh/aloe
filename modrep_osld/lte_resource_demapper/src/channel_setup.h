
#define ENABLE_PSS
#define ENABLE_SSS
#define ENABLE_REF

#define COPY_SIGNAL_PORT	1

#define EXTRACT_REF_PORT	-1

#define CH_OFF -1

struct channel pdsch[] = {
		{"PDSCH_0",	CH_PDSCH, 0, 0},
		{"PDSCH_1",	CH_PDSCH, 1, 0},
		{NULL,-1, -1,-1}
};

struct channel pdcch[] = {
		{"PDCCH_0", CH_PDCCH, 2, 0},
		{"PDCCH_1", CH_PDCCH, 3, 0},
		{NULL,-1, -1,-1}
};

struct channel other[] = {
		{"PCFICH", CH_PCFICH, 4, 0},
		{"PHICH", CH_PHICH, 5, 0},
		{"PBCH", CH_PBCH, 6, 0},
		{NULL,-1, -1,-1}
};


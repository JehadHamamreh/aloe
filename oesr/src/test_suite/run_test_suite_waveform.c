#include <stdlib.h>
#include "defs.h"
#include "str.h"
#include "oesr_man.h"
#include "waveform.h"
#include "nod_waveform.h"
#include "mempool.h"
#include "rtdal.h"

/* in a multi-processing environment, this file wont be included */
#include "nod_anode.h"

waveform_t waveform;

int print_modes(waveform_t *waveform) {
	int i;
	for (i=0;i<waveform->nof_modes;i++) {
		printf("\t%d:\t%s\n",i,waveform->modes[i].desc);
	}
	return 0;
}

int print_execinfo(waveform_t *waveform, int tslot_us) {
	int i;
	const char *t;
	int total_cpu=0, total_max_cpu=0;
	printf(" ========================= Execinfo: %s ==============\n\n",waveform->name);
	printf(" Name\t\t\t\t  Mean Exec (us)   Max Exec (us)   Max Exec (ts) Processor Id:Pos\n");
	for (i=0;i<waveform->nof_modules;i++) {
		if (strlen(waveform->modules[i].name)<7) {
			t="\t\t\t\t";
		} else if (strlen(waveform->modules[i].name)<15) {
			t="\t\t\t";
		} else if (strlen(waveform->modules[i].name)<23) {
			t="\t\t";
		} else {
			t="\t";
		}
		printf(" %s%s%16.2f%16d%16d%18d:%d\n",waveform->modules[i].name,t,
				waveform->modules[i].execinfo.mean_exec_us,
				waveform->modules[i].execinfo.max_exec_us,waveform->modules[i].execinfo.max_exec_ts,
				waveform->modules[i].processor_idx,waveform->modules[i].exec_position);
		total_cpu += waveform->modules[i].execinfo.t_exec[0].tv_usec;
		total_max_cpu += waveform->modules[i].execinfo.max_exec_us;
	}
	printf(" Total\t\t\t%11d (%.2f%%)\t Max: %d (%.2f%%)\n",total_cpu, (float) 100*total_cpu/tslot_us,
			total_max_cpu, (float) 100*total_max_cpu/tslot_us);
	return 0;
}


void *_run_main(void *arg) {
	int c;
	int tslen;
	int mode;
	rtdal_machine_t machine;

	rtdal_machine(&machine);
	tslen = machine.ts_len_us;

	//rtdal_task_print_sched();

	/* this will be done by the module */
	if (nod_anode_initialize(2)) {
		aerror("initializing node\n");
		return NULL;
	}

	/* from here, this is the oesr_man interface only */
	if (oesr_man_initialize(NULL,2)) {
		aerror("initializing oesr_man\n");
		return NULL;
	}

	memset(&waveform,0,sizeof(waveform_t));

	c=0;
	printf("\n\nList of commands:\n"
			"\t<l>\tLoad waveform\n"
			"\t<i>\tSet INIT\n"
			"\t<r>\tSet RUN\n"
			"\t<p>\tSet PAUSE\n"
			"\t<t>\tSet STEP\n"
			"\t<s>\tStop waveform\n"
			"\t<m>\tSet waveform mode\n"
			"\t<e>\tView execution time\n"
			"\n<Ctr+C>\tExit\n");
	waveform_status_t new_status;
	do {
		if (c != '\n') {
			printf("\n>> ");
			fflush(0);
		}
		c = getchar();
		new_status.cur_status = LOADED;
		switch((char) c) {
		case 'm':
			getchar();
			print_modes(&waveform);
			printf("\nEnter waveform mode (index): ");
			if (scanf("%d",&mode) == -1) {
				aerror("reading input\n");
				break;
			}
			printf("\nSwitching to '%s'...\n",waveform.modes[mode].desc);
			if (waveform_mode_set(&waveform,waveform.modes[mode].name)) {
				aerror("setting waveform mode\n");
				break;
			}
			break;
		case 'l':
			waveform_delete(&waveform);

			strcpy(waveform.model_file,arg);
			strcpy(waveform.name,arg);

			if (waveform_parse(&waveform,1)) {
				aerror("parsing waveform\n");
				return NULL;
			}

			if (waveform_load(&waveform)) {
				aerror("loading waveform\n");
				break;
			}
			fflush(stdout);
			printf("OK!\n");
			break;
		case 'i':
			new_status.cur_status=INIT;
			break;
		case 'r':
			new_status.cur_status=RUN;
			break;
		case 'p':
			new_status.cur_status=PAUSE;
			break;
		case 't':
			new_status.cur_status=STEP;
			break;
		case 's':
			new_status.cur_status=STOP;
			break;
		case 'e':
			if (waveform_update(&waveform)) {
				aerror("updating waveform\n");
				break;
			}
			if (print_execinfo(&waveform,tslen)) {
				aerror("printing execinfo\n");
				break;
			}
			break;
		case '\n':
			break;
		default:
			printf("Unknown command %c\n",(char) c);
			break;
		}
		if (new_status.cur_status != LOADED) {
			new_status.next_timeslot = rtdal_time_slot();
			if (waveform_status_set(&waveform,&new_status)) {
				printf("DID NOT CHANGE!\n");
			} else {
				printf("OK!\n");
			}
		}
	} while(1);
	/* status init */

	/* pause o sleep o return */

	printf("exit\n");
	return NULL;
}

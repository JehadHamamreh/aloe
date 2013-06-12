/*
 ============================================================================
 Name        : test_rt.c
 Author      : Ismael Gómez
 Version     :
 Copyright   : Copyright, 2013
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <error.h>
#include <complex.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __XENO__
#include <rtdk.h>
#endif

#include "rdtsc.h"

#define MAX_THREADS 5
#define DEFAULT_LOG_SIZE 100000
#define DEFAULT_NSAMPLES 100

#define DEFAULT_RTPRIO sched_get_priority_max(SCHED_FIFO)

int nof_threads;
int log_size;
int nof_operations;
int write_window;
int num_samples;
long int period;
int running;

typedef _Complex float data_t;

struct log_entry {
	long int timer_st;
	long int timer_end;
	long long *tsc;
};
typedef struct log {
	struct log_entry *mem;
	int wp;
	int size;
	int wrapped;
	int id;
}log_t;

log_t logs[MAX_THREADS];

void start_timer(struct timespec *now);
int wait_timer(struct timespec *now);
void timespec_add_us(struct timespec *t, long int us);

#ifdef __XENO__
#define r_printf rt_printf
#else
#define r_printf printf
#endif

void *worker(void *arg) {

#ifdef __XENO__
	//pthread_set_mode_np(0, PTHREAD_WARNSW);
#endif
	struct timespec now;
	log_t *log = (log_t*) arg;

	data_t *x,*y,*z;

	x=malloc(sizeof(data_t)*write_window);
	y=malloc(sizeof(data_t)*write_window);
	z=malloc(sizeof(data_t)*write_window);

	for (int i=0;i<write_window;i++) {
		x[i]=(float) rand()/INT_MAX+I*((float) rand()/INT_MAX);
		y[i]=(float) rand()/INT_MAX+I*((float) rand()/INT_MAX);
	}

	r_printf("starting periodic task with period %lu us\n",period);
	start_timer(&now);
	unsigned int counter=0;

	while(running) {
		if (wait_timer(&now)) {
			running=0;
			break;
		}

		clock_gettime(CLOCK_REALTIME,&now);
		log->mem[log->wp].timer_st = now.tv_nsec;

		for (int i=0;i<num_samples;i++) {
			log->mem[log->wp].tsc[i] = rdtsc();
			for (int j=0;j<write_window;j++) {
				z[j]=x[j]*y[j];
			}
		}

		if (!(counter%(1000000/period))) {
			r_printf("-- Run %u cycles\n",counter);
		}
		counter++;

		clock_gettime(CLOCK_REALTIME,&now);
		log->mem[log->wp].timer_end = now.tv_nsec;

		log->wp++;
		if (log->wp>=log->size) {
			log->wp-=log->size;
			log->wrapped++;
		}
	}
	return NULL;
}

int log_write(log_t *log) {
	char timerstname[64];
	char timerendname[64];
	char workname[64];
	FILE *timer_st_fd, *timer_end_fd, *work_fd;
	int rp,len;

	snprintf(timerstname,64,"timer_in_%d.log",log->id);
	snprintf(timerendname,64,"timer_out_%d.log",log->id);
	snprintf(workname,64,"work_%d.log",log->id);

	timer_st_fd = fopen(timerstname,"w");
	if (timer_st_fd == NULL) {
		perror("open");
		return -1;
	}
	timer_end_fd = fopen(timerendname,"w");
	if (timer_end_fd == NULL) {
		perror("open");
		return -1;
	}
	work_fd = fopen(workname,"w");
	if (work_fd == NULL) {
		perror("open");
		return -1;
	}
	if (log->wrapped) {
		rp=log->wp;
		len=log->size;
	} else {
		rp=0;
		len=log->wp;
	}
	printf("writting %d log positions\n",len);
	for (int i=rp;i<len;i++) {
		fwrite(&log->mem[i%log->size].timer_st,sizeof(long int),1,timer_st_fd);
		fwrite(&log->mem[i%log->size].timer_end,sizeof(long int),1,timer_end_fd);
		for (int j=0;j<num_samples;j++) {
			fwrite(&log->mem[i%log->size].tsc[j],sizeof(long long),1,work_fd);
		}
		free(log->mem[i%log->size].tsc);
	}
	free(log->mem);
	printf("Done\n");
	fclose(timer_st_fd);
	fclose(timer_end_fd);
	fclose(work_fd);
	return 0;
}

int launch_thread(pthread_t *thread, void *arg) {
	struct sched_param parm;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, 1);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	parm.sched_priority = DEFAULT_RTPRIO;
	pthread_attr_setschedparam(&attr, &parm);

	return pthread_create(thread, &attr, worker, arg);
}

int read_params(int argc, char **argv) {
	if (argc<4) {
		return -1;
	}
	period = atol(argv[1]);
	nof_threads = atoi(argv[2]);
	nof_operations=atoi(argv[3]);
	if (argc==5) {
		num_samples=atoi(argv[4]);
	} else {
		num_samples=DEFAULT_NSAMPLES;
	}
	write_window = nof_operations/num_samples;
	nof_operations=write_window*num_samples;
	if (argc==6) {
		log_size=atoi(argv[5]);
	} else {
		log_size=DEFAULT_LOG_SIZE;
	}
	if (nof_threads>MAX_THREADS) {
		printf("maximum threads is %d\n",MAX_THREADS);
		return -1;
	}
	return 0;
}

void usage(char *arg0) {
	printf("usage: %s period_us num_threads num_operations [nsamples] [log_size]\n",arg0);
}

int main(int argc, char **argv) {
	pthread_t workers[MAX_THREADS];

	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (read_params(argc,argv)) {
		usage(argv[0]);
		exit(0);
	}
	printf("Allocating memory for log...\n");
	for (int i=0;i<nof_threads;i++) {
		logs[i].id=i;
		logs[i].wrapped=0;
		logs[i].wp=0;
		logs[i].size=log_size;
		logs[i].mem=calloc(log_size,sizeof(struct log_entry));
		for (int j=0;j<log_size;j++) {
			logs[i].mem[j].tsc = calloc(num_samples,sizeof(long long));
		}
	}
	sigset_t mask;
	int sig;
	sigemptyset(&mask);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGTERM);
	sigaddset(&mask,SIGHUP);
	sigaddset(&mask,SIGALRM);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);

	printf("Launching %d threads running %d operations (%dx%d)...\n",
			nof_threads,nof_operations,num_samples,write_window);

	running=1;
	for (int i=0;i<nof_threads;i++) {
		if (launch_thread(&workers[i],&logs[i])) {
			printf("Error launching thread %d\n",i);
			exit(0);
		}
	}
	sigwait(&mask, &sig);
	running=0;
	for (int i=0;i<nof_threads;i++) {
		pthread_join(workers[i], NULL);
	}
	for (int i=0;i<nof_threads;i++) {
		if (log_write(&logs[i])) {
			printf("Error writting log %d\n",i);
		}
	}
	return EXIT_SUCCESS;
}



inline void start_timer(struct timespec *now) {
	clock_gettime(CLOCK_REALTIME,now);
	now->tv_sec++;

#ifdef __XENO__
	struct timespec xperiod;
	xperiod.tv_sec=0;
	xperiod.tv_nsec=period*1000;
	int s;
	if ((s=pthread_make_periodic_np(pthread_self(), now, &xperiod))) {
		printf("error %d\n",s);
	}
#endif
}

inline void timespec_add_us(struct timespec *t, long int us) {
	t->tv_nsec += us;
	if (t->tv_nsec >= 1000000000) {
		t->tv_nsec = t->tv_nsec - 1000000000;
		t->tv_sec += 1;
	}
}

inline int wait_timer(struct timespec *now) {
#ifdef __XENO__
	unsigned long overruns=0;
	int s;
	if ((s=pthread_wait_np(&overruns))) {
		if (s!=110) {
			printf("error2 %d\n",s);
		}
	}
	if (overruns>0) {
		r_printf("overruns=%lu\n",overruns);
		return -1;
	}
#else
	timespec_add_us(now, period);
	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, now, NULL);
#endif
	return 0;
}

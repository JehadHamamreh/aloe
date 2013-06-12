#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include "rtdal.h"
#include "futex.h"
#include "objects_max.h"
#include "pipeline_sync.h"
#include "defs.h"

static int num_pipelines;

static sem_t semaphores[MAX_PIPELINES];
static int futex;
static pthread_barrier_t barrier;

static pthread_mutex_t sync_mutex;
static pthread_cond_t sync_cond;
int timeslot = 0;
int timeslot_p[MAX_PIPELINES];

static inline int pipeline_sync_initialize_condvar() {
	pthread_mutex_init(&sync_mutex, NULL);
	pthread_cond_init(&sync_cond, NULL);
	memset(timeslot_p, 0, sizeof(int) * MAX_PIPELINES);
	return 0;
}
static inline void pipeline_sync_thread_waits_condvar(int idx) {
	hdebug("timeslot=%d, mytslot=%d, idx=%d\n",timeslot,timeslot_p[idx],idx);
	pthread_mutex_lock(&sync_mutex);
	while (timeslot <= timeslot_p[idx]) {
		pthread_cond_wait(&sync_cond, &sync_mutex);
	}
	pthread_mutex_unlock(&sync_mutex);
	timeslot_p[idx] = timeslot;
	hdebug("timeslot=%d, mytslot=%d, idx=%d\n",timeslot,timeslot_p[idx],idx);
}
static inline void pipeline_sync_thread_wake_condvar() {
	pthread_mutex_lock(&sync_mutex);
	timeslot++;
	pthread_cond_broadcast(&sync_cond);
	pthread_mutex_unlock(&sync_mutex);
	hdebug("timeslot=%d\n",timeslot);
}

static inline int pipeline_sync_initialize_sem() {
	for (int i = 0; i < num_pipelines; i++) {
		sem_init(&semaphores[i], 0, 0);
	}
	return 0;
}

static inline void pipeline_sync_thread_waits_sem(int idx) {
	sem_wait(&semaphores[idx]);
}

static inline void pipeline_sync_thread_wake_sem() {
	for (int i = 0; i < num_pipelines; i++) {
		sem_post(&semaphores[i]);
	}
}

static inline void pipeline_sync_thread_wake_sem_idx(int idx) {
	sem_post(&semaphores[idx]);
}

static inline int pipeline_sync_initialize_futex() {
	return 0;
}
static inline void pipeline_sync_thread_waits_futex(int idx) {
	futex_wait(&futex);
}
static inline void pipeline_sync_thread_wake_futex() {
	futex_wake(&futex);
}

static inline int pipeline_sync_initialize_barrier() {
	pthread_barrier_init(&barrier, NULL, (unsigned int) num_pipelines+1);
	return 0;
}
static inline void pipeline_sync_thread_waits_barrier(int idx) {
	pthread_barrier_wait(&barrier);
}
static inline void pipeline_sync_thread_wake_barrier() {
	pthread_barrier_wait(&barrier);
}

inline int pipeline_sync_initialize(int _num_pipelines) {
	if (_num_pipelines > MAX_PIPELINES) {
		return -1;
	}
	num_pipelines = _num_pipelines;

#if PIPELINESYNC_MUTEX_TYPE==SEM
	return pipeline_sync_initialize_sem();
#endif
#if PIPELINESYNC_MUTEX_TYPE==FUTEX
	return pipeline_sync_initialize_futex();
#endif
#if PIPELINESYNC_MUTEX_TYPE==BARRIER
	return pipeline_sync_initialize_barrier();
#endif
#if PIPELINESYNC_MUTEX_TYPE==CONDVAR
	return pipeline_sync_initialize_condvar();
#endif

}

inline void pipeline_sync_thread_waits(int idx) {

#if PIPELINESYNC_MUTEX_TYPE==SEM
	pipeline_sync_thread_waits_sem(idx);
#endif
#if PIPELINESYNC_MUTEX_TYPE==FUTEX
	pipeline_sync_thread_waits_futex(idx);
#endif
#if PIPELINESYNC_MUTEX_TYPE==BARRIER
	pipeline_sync_thread_waits_barrier(idx);
#endif
#if PIPELINESYNC_MUTEX_TYPE==CONDVAR
	pipeline_sync_thread_waits_condvar(idx);
#endif
}

inline void pipeline_sync_threads_wake() {
#if PIPELINESYNC_MUTEX_TYPE==SEM
	pipeline_sync_thread_wake_sem();
#endif
#if PIPELINESYNC_MUTEX_TYPE==FUTEX
	pipeline_sync_thread_wake_futex();
#endif
#if PIPELINESYNC_MUTEX_TYPE==BARRIER
	pipeline_sync_thread_wake_barrier();
#endif
#if PIPELINESYNC_MUTEX_TYPE==CONDVAR
	pipeline_sync_thread_wake_condvar();
#endif
}

inline void pipeline_sync_threads_wake_idx(int idx) {
#if PIPELINESYNC_MUTEX_TYPE==SEM
	pipeline_sync_thread_wake_sem_idx(idx);
#endif
#if PIPELINESYNC_MUTEX_TYPE==FUTEX
#error Not implemented
#endif
#if PIPELINESYNC_MUTEX_TYPE==BARRIER
	pipeline_sync_thread_wake_barrier();
#endif
#if PIPELINESYNC_MUTEX_TYPE==CONDVAR
	pipeline_sync_thread_wake_condvar();
#endif
}



#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef __XENO__
#include <rtdk.h>
#endif

#include "rtdal.h"
#include "defs.h"
#include "rtdal_error.h"

typedef struct {
	int id;
	char *memory;
	char *tmp;
	lstrdef(name);
	int size;
	int opts;
	int wrapped;
	pthread_mutex_t mutex;
	int wpm;
	r_log_mode_t mode;
}log_t;

#if LOGS_ENABLED!=0

static lstrdef(base_path);
static int max_str_len;

static log_t *logs;
static int max_logs;
static int logs_enabled=0;
static int default_log_sz;
static FILE *output;
static pthread_mutex_t mutex;
#endif

#define cast(a,b) CAST(a,b,log_t*)
#define lock()	do {if(log->opts&RTDAL_LOG_OPTS_EXCL) pthread_mutex_lock(&log->mutex);}while(0)
#define unlock()	do {if(log->opts&RTDAL_LOG_OPTS_EXCL) pthread_mutex_unlock(&log->mutex);}while(0)


int rtdal_log_init(char *_base_path, int _max_logs, int _max_str_len, int _default_log_sz, void *redirect_stream) {
#if LOGS_ENABLED!=0
	lstrcpy(base_path,_base_path);

	max_str_len = _max_str_len;
	max_logs = _max_logs;
	logs = calloc(max_logs,sizeof(log_t));
	if (!logs) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		return -1;
	}
	pthread_mutex_init(&mutex,NULL);
	logs_enabled=1;
	default_log_sz = _default_log_sz;
	output = redirect_stream;
#ifdef __XENO__
	rt_print_auto_init(1);
#endif
#else
	aerror("Logs are disabled\n");
#endif
	return 0;
}

void rtdal_log_flushall() {
#if LOGS_ENABLED!=0
	int i;
	if (!logs_enabled) {
		return;
	}

	for (i=0;i<max_logs;i++) {
		if (logs[i].id) {
			rtdal_log_flush((r_log_t) &logs[i]);
		}
	}
#endif
}

void rtdal_log_flush(r_log_t _log) {
#if LOGS_ENABLED!=0
	cast(log,_log);
	assert(log);
	if (!logs_enabled) {
		return;
	}
	pthread_mutex_lock(&mutex);

	FILE *file;
	snprintf(log->tmp,max_str_len,"%s/%s",base_path,log->name);
	if (log->mode == TEXT) {
		file = fopen(log->tmp,"w+");
	} else {
		file = fopen(log->tmp,"wb+");
	}

	if (!file) {
		aerror_msg("Error opening file %s: ",log->tmp);
		perror("fopen");
		pthread_mutex_unlock(&mutex);
		return;
	}
	printf("Writting to log file %s...",log->tmp);fflush(stdout);
	if (log->wrapped) {
		if (fwrite(&log->memory[log->wpm],1,log->size-log->wpm,file) == -1) {
			aerror_msg("Error writing file %s: ",log->tmp);
			perror("fwrite");
		}
	}
	if (fwrite(&log->memory[0],1,log->wpm,file) == -1) {
		aerror_msg("Error writing file %s: ",log->tmp);
		perror("fwrite");
	}
	printf("done\n");
	fflush(file);
	fclose(file);
	pthread_mutex_unlock(&mutex);
#endif
}

void rtdal_log_delete(r_log_t _log) {
#if LOGS_ENABLED!=0
	if (!logs_enabled) {
		return;
	}
	pthread_mutex_lock(&mutex);
	cast(log,_log);
	assert(log);
	if (log->memory) {
		free(log->memory);
	}
	if (log->tmp) {
		free(log->tmp);
	}
	pthread_mutex_unlock(&mutex);
#endif
}


r_log_t rtdal_log_new_opts(char *name, r_log_mode_t mode, int size, int opts) {
#if LOGS_ENABLED!=0
	if (!logs_enabled) {
		return NULL;
	}
	pthread_mutex_lock(&mutex);
	assert(name);
	assert(size>=0);
	int i=0;
	while(logs[i].id && i<max_logs) {
		i++;
	}
	if (i==max_logs) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	if (!size) {
		size = default_log_sz;
	}

	logs[i].memory = calloc(size,sizeof(char));
	if (!logs[i].memory) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	logs[i].tmp = calloc(max_str_len,sizeof(char));
	if (!logs[i].tmp) {
		RTDAL_SETERROR(RTDAL_ERROR_NOSPACE);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	lstrcpy(logs[i].name,name);
	logs[i].opts = opts;
	if (opts & RTDAL_LOG_OPTS_EXCL) {
		pthread_mutex_init(&logs[i].mutex,NULL);
	}
	logs[i].wpm = 0;
	logs[i].size = size;
	logs[i].mode = mode;
	logs[i].wrapped = 0;
	logs[i].id = i+1;
	pthread_mutex_unlock(&mutex);

	return (r_log_t) &logs[i];
#else
	aerror("Logs are disabled\n");
	return NULL;
#endif
}

r_log_t rtdal_log_new(char *name, r_log_mode_t mode, int size) {
	return rtdal_log_new_opts(name, mode, size, 0);
}

void rtdal_log_add_(r_log_t _log, void *_data, int size, int locked) {
#if LOGS_ENABLED!=0
	if (!logs_enabled) {
		return;
	}
	cast(log,_log);
	if (!log) {
		return;
	}
	if (!locked) {
		lock();
	}
	char *data = _data;
	assert(log);
	assert(data);

	int blen=size;

	if (size > log->size) {
		unlock();
		return;
	}

	if (blen > log->size-log->wpm) {
		blen=log->size-log->wpm;
	}
	memcpy(&log->memory[log->wpm],&data[0],blen*sizeof(char));
	if (size-blen>0) {
		memcpy(&log->memory[0],&data[blen],(size-blen)*sizeof(char));
	}
	log->wpm+=size;
	if (log->wpm>log->size) {
		log->wrapped++;
		log->wpm-=log->size;
	}
	if (!locked) {
		unlock();
	}
#endif
}
void rtdal_log_add(r_log_t _log, void *_data, int size) {
	rtdal_log_add_(_log, _data, size, 0);
}

void rtdal_log_vprintf(r_log_t _log, const char *format, va_list ap) {
#if LOGS_ENABLED!=0
	if (!logs_enabled) {
		return;
	}
	cast(log,_log);
	if (!log) {
		return;
	}
	lock();
	assert(log);
	assert(format);

	int len;

	vsnprintf(log->tmp,max_str_len,format,ap);

	len = strnlen(log->tmp,max_str_len);

	rtdal_log_add_((r_log_t) log,log->tmp,len,1);


	if (output) {
#ifdef __XENO__
		rt_vfprintf(output,format,ap);
#else
		vfprintf(output,format,ap);
#endif
	}
	unlock();
#endif
}

void rtdal_log_printf(r_log_t _log, const char *format, ... ) {
	va_list ap;
	va_start(ap,format);
	rtdal_log_vprintf(_log,format,ap);
}

void rtdal_log_add_us(r_log_t _log) {
#if LOGS_ENABLED!=0
	if (!logs_enabled) {
		return;
	}
	cast(log,_log);
	if (!log) {
		return;
	}
	lock();
	assert(log);

	struct timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	unsigned int usec_u;
	int usec_s;

	switch (log->mode) {
	case UINT32:
		usec_u = (unsigned int) t.tv_nsec/1000;
		rtdal_log_add(_log,(char*)&usec_u,sizeof(unsigned int));
		break;
	case INT32:
		usec_s = (int) t.tv_nsec/1000;
		rtdal_log_add(_log,(char*)&usec_s,sizeof(int));
		break;

	case TEXT:
		snprintf(log->tmp,max_str_len,"%u, ",(unsigned int) t.tv_nsec/1000);
		rtdal_log_add(_log,log->tmp,strlen(log->tmp));
		break;
	}
	unlock();
#endif
}

void rtdal_log_add_tslot(r_log_t _log) {
#if LOGS_ENABLED!=0
	if (!logs_enabled) {
		return;
	}
	cast(log,_log);
	lock();
	assert(log);

	int tslot = rtdal_time_slot();
	unsigned int tslot_u;

	switch (log->mode) {
	case UINT32:
		tslot_u = (unsigned int) tslot;
		rtdal_log_add(_log,(char*)&tslot_u,sizeof(unsigned int));
		break;
	case INT32:
		rtdal_log_add(_log,(char*)&tslot,sizeof(int));
		break;
	case TEXT:
		snprintf(log->tmp,max_str_len,"%d, ",tslot);
		rtdal_log_add(_log,log->tmp,strlen(log->tmp));
		break;
	}
	unlock();
#endif
}


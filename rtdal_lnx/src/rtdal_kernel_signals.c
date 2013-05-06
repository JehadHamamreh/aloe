#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <execinfo.h>
#include <unistd.h>
#include <dlfcn.h>
#if !defined(__cplusplus) && !defined(NO_CPP_DEMANGLE)
#define NO_CPP_DEMANGLE
#endif

#include "rtdal_context.h"
#include "rtdal_kernel.h"
#include "str.h"
#include "defs.h"


#define KERNEL_SIG_THREAD_SPECIFIC SIGRTMIN
#define N_THREAD_SPECIFIC_SIGNALS 8
const int thread_specific_signals[N_THREAD_SPECIFIC_SIGNALS] =
	{SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE, SIGXCPU, TASK_TERMINATION_SIGNAL, SIGUSR1};
const char *thread_specific_signals_name[N_THREAD_SPECIFIC_SIGNALS] =
	{"SIGSEGV", "SIGABRT", "SIGBUS", "SIGILL", "SIGFPE", "SIGXCPU","TASK_TERMINATION_SIGNAL", "SIGUSR1"};

extern int sigwait_stops;
strdef(tmp_msg);
extern rtdal_context_t rtdal;
extern int signal_received;
extern pthread_t single_timer_thread;
extern pid_t kernel_pid;

/**
 * called after kernel timer caught a synchronous signal.
 * todo: try to recover thread?
 */
static void kernel_timer_recover_thread() {
	aerror("Not implemented\n");
}

/*#define RECOVER_THREAD
*/

static int sigwait_loop_process_thread_signal(siginfo_t *info) {
	int thread_id, signum;

	signum = info->si_value.sival_int & 0xffff;
	thread_id = info->si_value.sival_int >> 16;
	hdebug("signum=%d, thread_id=%d\n",signum,thread_id);
	if (signum < N_THREAD_SPECIFIC_SIGNALS) {
		snprintf(tmp_msg, STR_LEN, "[rtdal]: Got signal %s from ",
				thread_specific_signals_name[signum]);
	} else {
		snprintf(tmp_msg, STR_LEN, "[rtdal]: Got unknown signal from ");
	}

	/* now try to restore the pipeline, if the thread was a pipeline */
	if (thread_id > -1) {
		if (strnlen(tmp_msg,STR_LEN)>1) {
			snprintf(&tmp_msg[strlen(tmp_msg)-1],STR_LEN,
					" pipeline thread %d. Current running process idx is %d\n",
					thread_id,
					rtdal.pipelines[thread_id].running_process_idx);
		}
#ifdef RECOVER_THREAD
		if (pipeline_recover_thread(
				&rtdal.pipelines[thread_id])) {
			aerror("recovering pipeline thread\n");
		}
#endif

	} else if (info->si_value.sival_int == -1) {
		strcat(tmp_msg, " the kernel thread\n");
		kernel_timer_recover_thread();
	} else {
		strcat(tmp_msg, " an unkown thread\n");
	}
	write(0,tmp_msg,strlen(tmp_msg));
	return 1;
}

/*#define EXIT_ON_THREADSIG
*/
/**
 * This is a thread with priority kernel_prio that synchronously waits for
 * rtdal_pipeline signals (usign sigwaitinfo). All signals except thread-specific
 * ones (SIGSEGV,SIGILL,SIGBUS,SIGFPE) are blocked by all threads except this one.
 * Thread-specific signals are handled by ProcThreads which send a SIGRTMIN+1,
 * SIGRTMIN+2,SIGRTMIN+3,SIGRTMIN+4 (respectively) to this thread, which takes
 * actions accordingly.
 *
 * for signals SIGRTMIN to SIGRTMIN+4, cast the rtdal_pipeline object from this
 * si_value pointer and call rtdal_pipeline_recover_thread(pipeline,
 * pipeline->running_process, TRUE)
 */
void sigwait_loop(void) {

	int signum;
	sigset_t set;
	siginfo_t info;

	sigfillset(&set);
	sigdelset(&set,TASK_TERMINATION_SIGNAL);
	while(!sigwait_stops) {
		do {
			signum = sigwaitinfo(&set, &info);
		} while (signum == -1 && errno == EINTR);
		if (signum == -1) {
			poserror(errno, "sigwaitinfo");
			return;
		}
		hdebug("detected signal %d\n",signum);
		if (signum == KERNEL_SIG_THREAD_SPECIFIC) {
			printf("[rtdal]: Caught thread-specific signal\n");
#ifdef EXIT_ON_THREADSIG
			fflush(stdout);
			return;
#else
			sigwait_loop_process_thread_signal(&info);
#endif
		} else if (signum == SIGINT) {
			printf("Caught SIGINT, exiting\n");
			fflush(stdout);
			return;
		} else if (signum != SIGWINCH && signum != SIGCHLD) {
			printf("Got signal %d, exiting\n", signum);
			fflush(stdout);
			return;
		}
	}

}

static void full_write(int fd, const char *buf, size_t len)
{
        while (len > 0) {
                ssize_t ret = write(fd, buf, len);

                if ((ret == -1) && (errno != EINTR))
                        break;

                buf += (size_t) ret;
                len -= (size_t) ret;
        }
}

void *bt[1024];

/** FIXME: Use REG_RIP on IA64 */
void print_backtrace(void)
{
        static const char start[] = "BACKTRACE ------------\n";
        static const char end[] = "----------------------\n";

        int bt_size;
        char **bt_syms;
        int i;

        bt_size = backtrace(bt, 1024);
        bt_syms = backtrace_symbols(bt, bt_size);
        full_write(STDERR_FILENO, start, strlen(start));
        for (i = 2; i < bt_size; i++) {
                size_t len = strlen(bt_syms[i]);
                full_write(STDERR_FILENO, bt_syms[i], len);
                full_write(STDERR_FILENO, "\n", 1);
        }
        full_write(STDERR_FILENO, end, strlen(end));
    free(bt_syms);
}

#define sigsegv_outp(x, ...)    fprintf(stderr, x "\n", ##__VA_ARGS__)

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif

static void signal_segv(int signum, siginfo_t* info, void*ptr)
{
	static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};

	int i, f = 0;
	ucontext_t *ucontext = (ucontext_t*)ptr;
	Dl_info dlinfo;
	void **bp = 0;
	void *ip = 0;

	sigsegv_outp("Segmentation Fault!");
	sigsegv_outp("info.si_signo = %d", signum);
	sigsegv_outp("info.si_errno = %d", info->si_errno);
	sigsegv_outp("info.si_code  = %d (%s)", info->si_code, si_codes[info->si_code]);
	sigsegv_outp("info.si_addr  = %p", info->si_addr);
	for (i = 0; i < NGREG; i++)
		sigsegv_outp("reg[%02d]       = 0x" REGFORMAT, i, ucontext->uc_mcontext.gregs[i]);

#ifndef SIGSEGV_NOSTACK
#if defined(SIGSEGV_STACK_IA64) || defined(SIGSEGV_STACK_X86)
#if defined(SIGSEGV_STACK_IA64)
	ip = (void*)ucontext->uc_mcontext.gregs[REG_RIP];
	bp = (void**)ucontext->uc_mcontext.gregs[REG_RBP];
#elif defined(SIGSEGV_STACK_X86)
	ip = (void*)ucontext->uc_mcontext.gregs[REG_EIP];
	bp = (void**)ucontext->uc_mcontext.gregs[REG_EBP];
#endif

	sigsegv_outp("Stack trace:");
	while (bp && ip) {
		if (!dladdr(ip, &dlinfo))
			break;

		const char *symname = dlinfo.dli_sname;

#ifndef NO_CPP_DEMANGLE
		int status;
		char * tmp = __cxa_demangle(symname, NULL, 0, &status);

		if (status == 0 && tmp)
			symname = tmp;
#endif

		sigsegv_outp("% 2d: %p <%s+%lu> (%s)",
			     ++f,
			     ip,
			     symname,
			     (unsigned long)ip - (unsigned long)dlinfo.dli_saddr,
			     dlinfo.dli_fname);

#ifndef NO_CPP_DEMANGLE
		if (tmp)
			free(tmp);
#endif

		if (dlinfo.dli_sname && !strcmp(dlinfo.dli_sname, "main"))
			break;

		ip = bp[1];
		bp = (void**)bp[0];
	}
#else
	sigsegv_outp("Stack trace (non-dedicated):");
	sz = backtrace(bt, 20);
	strings = backtrace_symbols(bt, sz);
	for (i = 0; i < sz; ++i)
		sigsegv_outp("%s", strings[i]);
#endif
	sigsegv_outp("End of stack trace.");
#else
	sigsegv_outp("Not printing stack strace.");
#endif
	_exit(-1);
}

#define PRINT_BT_ON_SIGSEGV

/**
 * Handler for thread-specific signals (SIGSEGV,SIGILL,SIGFPE,SIGBUS).
 * Forwards a signal above SIGRTMIN to myself. Since it is blocked, it will
 * be received by sigwait_loop(), which is runs in the main kernel thread.
 *
 * The thread terminates after exiting the handler.
 */
void thread_signal_handler(int signum, siginfo_t *info, void *ctx) {
	union sigval value;
	int thread_id;
	int i;

	if (signum == SIGXCPU) {
		write(1,"-",1);
		return;
	}

#ifdef PRINT_BT_ON_SIGSEGV
	if (signum == SIGSEGV) {
		signal_segv(signum,info,ctx);
	}
#endif

	signal_received++;
	thread_id = -1;


	/* try to find the thread that caused the signal */

	/** todo: Caution!! is pthread_self() safe in the handler?
	 * it is not async-signal-safe by the standard,
	 * but the signals are synchronous.
	 */
	pthread_t thisthread = pthread_self();

	/* if signum is SIGUSR2, its a task termination signal, just exit */
	if (signum == TASK_TERMINATION_SIGNAL) {
		aerror_msg("sigusr2 signal. thread=%d\n",thisthread);
		goto cancel_and_exit;
	}

	/* is it a pipeline thread? */
	for (i=0;i<rtdal.machine.nof_cores;i++) {
		if (thisthread == rtdal.pipelines[i].thread) {
			break;
		}
	}
	if (i < rtdal.machine.nof_cores) {
		aerror_msg("pipeline_idx=%d\n",i);
		thread_id = i;

		/* set the thread to 0 because is terminating */
		rtdal.pipelines[thread_id].thread = 0;
	} else {
		/* it is not, may be it is the kernel timer */
		if (thisthread == single_timer_thread) {
			aerror_msg("timer thread=%d\n",thisthread);
			thread_id = -1;
		} else {
			/* @TODO: check if it is a status or init thread of any module */

			aerror_msg("other thread=%d\n",thisthread);
			goto cancel_and_exit;
		}
	}

	/* Now send a signal to the kernel */
	for (i=0;i<N_THREAD_SPECIFIC_SIGNALS;i++) {
		if (thread_specific_signals[i] == signum)
			break;
	}
	aerror_msg("signal=%s, thread=%d\n",thread_specific_signals_name[i],thread_id);
	value.sival_int = thread_id<<16 | i;
	if (sigqueue(kernel_pid,
			KERNEL_SIG_THREAD_SPECIFIC,
			value)) {
		poserror(errno, "sigqueue");
	}


cancel_and_exit:
	if (signum != SIGABRT || thread_id == -1) {
		pthread_exit(NULL);
	} else {
		rtdal.pipelines[thread_id].waiting=1;
		while(rtdal.pipelines[thread_id].waiting) {
			usleep(1000);
		}
	}
}


/**
 * Block all signals except (SIGSEGV,SIGILL,SIGFPE,SIGBUS),
 * which sigwaited by the sigwait_thread
 */
int kernel_initialize_setup_signals() {
	int s;
	sigset_t set;
	int i;
	struct sigaction action;

	action.sa_sigaction = thread_signal_handler;
	action.sa_flags = SA_SIGINFO;

	sigfillset(&set);
	/* do not block thread-specific signals. Add a handler to them */
	for (i=0;i<N_THREAD_SPECIFIC_SIGNALS;i++) {
			sigdelset(&set, thread_specific_signals[i]);
			if (sigaction(thread_specific_signals[i], &action, NULL)) {
				poserror(errno, "sigaction");
				return -1;
			}
	}
	s = sigprocmask(SIG_BLOCK, &set, NULL);
	if (s != 0) {
		poserror(errno, "sigprocmask");
		return -1;
	}
	return 0;
}



#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "rtdal_context.h"
#include "rtdal_kernel.h"
#include "str.h"
#include "defs.h"


#define KERNEL_SIG_THREAD_SPECIFIC SIGRTMIN
#define N_THREAD_SPECIFIC_SIGNALS 6
const int thread_specific_signals[N_THREAD_SPECIFIC_SIGNALS] =
	{SIGSEGV, SIGBUS, SIGILL, SIGFPE, TASK_TERMINATION_SIGNAL, SIGUSR1};

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


static int sigwait_loop_process_thread_signal(siginfo_t *info) {
	int thread_id, signum;

	signum = info->si_value.sival_int & 0xffff;
	thread_id = info->si_value.sival_int >> 16;
	hdebug("signum=%d, thread_id=%d\n",signum,thread_id);
	if (signum < N_THREAD_SPECIFIC_SIGNALS) {
		snprintf(tmp_msg, STR_LEN, "Got signal num %d from ",
				thread_specific_signals[signum]);
	} else {
		snprintf(tmp_msg, STR_LEN, "Got unknown signal from ");
	}

	/* now try to restore the pipeline, if the thread was a pipeline */
	if (thread_id > -1) {
		if (strnlen(tmp_msg,STR_LEN)>1) {
			snprintf(&tmp_msg[strlen(tmp_msg)-1],STR_LEN,
					"pipeline thread %d process %d\n",
					thread_id,
					rtdal.pipelines[thread_id].running_process_idx);
		}
		if (pipeline_recover_thread(
				&rtdal.pipelines[thread_id])) {
			aerror("recovering pipeline thread\n");
		}

	} else if (info->si_value.sival_int == -1) {
		strcat(tmp_msg, "the kernel thread\n");
		kernel_timer_recover_thread();
	} else {
		strcat(tmp_msg, "an unkown thread\n");
	}
	return 1;
}

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
			goto out;
		}
		hdebug("detected signal %d\n",signum);
		if (signum == KERNEL_SIG_THREAD_SPECIFIC) {
			sigwait_loop_process_thread_signal(&info);
		} else if (signum == SIGINT) {
			printf("Caught SIGINT, exiting\n");
			fflush(stdout);
			goto out;
		} else if (signum != SIGWINCH && signum != SIGCHLD) {
			printf("Got signal %d, exiting\n", signum);
			fflush(stdout);
			goto out;
		}
	}

out:
	kernel_exit();
}


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

#ifdef PRINT_BT_ON_SIGSEGV
	void *pnt = NULL;
	ucontext_t *context = (ucontext_t*) ctx;
	char **msg;

	pnt = (void*) context->uc_mcontext.gregs[REG_EIP];
	msg = backtrace_symbols(&pnt, 1);
	strcat(sigmsg, msg[0]);
	free(msg);
#endif

	signal_received++;

	hdebug("[ts=%d] signal %d received\n",rtdal_time_slot(),signum);

	/* try to find the thread that caused the signal */

	/** todo: Caution!! is pthread_self() safe in the handler?
	 * it is not async-signal-safe by the standard,
	 * but the signals are synchronous.
	 */
	pthread_t thisthread = pthread_self();

	/* if signum is SIGUSR2, its a task termination signal, just exit */
	if (signum == TASK_TERMINATION_SIGNAL) {
		hdebug("sigusr2 signal. thread=%d\n",thisthread);
		goto cancel_and_exit;
	}

	/* is it a pipeline thread? */
	for (i=0;i<rtdal.machine.nof_cores;i++) {
		if (thisthread == rtdal.pipelines[i].thread) {
			break;
		}
	}
	if (i < rtdal.machine.nof_cores) {
		hdebug("pipeline_idx=%d\n",i);
		thread_id = i;

		/* set the thread to 0 because is terminating */
		rtdal.pipelines[thread_id].thread = 0;
	} else {
		/* it is not, may be it is the kernel timer */
		if (thisthread == single_timer_thread) {
			hdebug("timer thread=%d\n",thisthread);
			thread_id = -1;
		} else {
			/* @TODO: check if it is a status or init thread of any module */

			hdebug("other thread=%d\n",thisthread);
			goto cancel_and_exit;
		}
	}

	/* Now send a signal to the kernel */
	for (i=0;i<N_THREAD_SPECIFIC_SIGNALS;i++) {
		if (thread_specific_signals[i] == signum)
			break;
	}
	hdebug("signal=%d, thread=%d\n",i,thread_id);
	value.sival_int = thread_id<<16 | i;
	if (sigqueue(kernel_pid,
			KERNEL_SIG_THREAD_SPECIFIC,
			value)) {
		poserror(errno, "sigspscq");
	}

cancel_and_exit:
	pthread_exit(NULL);
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



/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef rtdal_KERNEL_H
#define rtdal_KERNEL_H

//#define KERNEL_DEB_TIME

#define KERNEL_RT_PRIO sched_get_priority_max(SCHED_FIFO)

#define ALOE_VERSION "ALOE++-0.6"
#define ALOE_YEAR 2013

#include "pipeline.h"

#define TASK_TERMINATION_SIGNAL	SIGUSR2

int kernel_tslot_run();
int rtdal_kernel_sigwait_thread();
int kernel_initialize_create_pipeline(pipeline_t *obj, int *wait_futex);
void kernel_cycle(void *x, struct timespec *time);
void dac_cycle(void);
void thread_signal_handler(int signum, siginfo_t *info, void *ctx);
void kernel_exit();
int parse_config(char *config_file, rtdal_machine_t *machine);
void sigwait_loop(void);
int kernel_initialize_setup_signals();

#endif

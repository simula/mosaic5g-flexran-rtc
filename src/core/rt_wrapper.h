/*
 * Copyright 2016-2018 FlexRAN Authors, Eurecom and The University of Edinburgh
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * For more information about Mosaic5G:  contact@mosaic-5g.io
 */

/*! \file    rt_wrapper.h
 *  \brief   helper for setting RT functionality
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef RT_WRAPPER_H_
#define RT_WRAPPER_H_

#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <syscall.h>
#include <math.h>

namespace flexran {

  namespace core {

    namespace rt {
    
      void set_latency_target(void);
      
      typedef long long int rtime;
    
//#define rt_printk printf
    
      rtime rt_get_time_ns (void);

      int rt_sleep_ns (rtime x);
    
      void check_clock(void);

#ifdef LOWLATENCY

#define SCHED_DEADLINE  6

    /* XXX use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr   314
#define __NR_sched_getattr   315
#endif

#ifdef __i386__
#define __NR_sched_setattr   351
#define __NR_sched_getattr   352
#endif

      struct sched_attr {
	__u32 size;
	
	__u32 sched_policy;
	__u64 sched_flags;
	
	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;
	
	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;
      
	/* SCHED_DEADLINE (nsec) */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
      };
    
      int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);
    
      int sched_getattr(pid_t pid,struct sched_attr *attr,unsigned int size, unsigned int flags);

#endif
      
    }
    
  }

}

#endif

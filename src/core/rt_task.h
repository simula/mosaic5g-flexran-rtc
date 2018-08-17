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

/*! \file    rt_task.h
 *  \brief   represents a task that runs an app etc
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef RT_TASK_H_
#define RT_TASK_H_

#include <linux/types.h>
#include "rt_wrapper.h"

typedef __u64 sched_time;
typedef __u32 sched_priority;

namespace flexran {

  namespace core {

    namespace rt {

      class rt_task {
	
      public:
	
#ifdef LOWLATENCY
	
	enum class Policy {DEFAULT = SCHED_OTHER, RR = SCHED_RR, DEADLINE = SCHED_DEADLINE, FIFO = SCHED_FIFO};
	
#else

	enum class Policy {NORMAL = 0, RR = 1, DEADLINE = 2, FIFO = 3};
	
#endif
  
	rt_task(Policy pol, sched_priority priority = 0, sched_time runtime = 0,
            sched_time deadline = 0, sched_time period = 0);
	
	void execute_task();
	
      private:
	
	virtual void run() = 0; 
	
#ifdef LOWLATENCY
  
	void set_scheduling_policy(Policy pol, sched_priority priority,
            sched_time runtime, sched_time deadline, sched_time period);
  
	struct sched_attr attr_;

#endif
	
      };
      
    }
    
  }

}
#endif

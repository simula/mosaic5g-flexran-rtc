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

/*! \file    rt_task.cc
 *  \brief   represents a task that runs an app etc
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#include <iostream>
#include <cerrno>
#include <cstring>

#include "rt_task.h"
#include "flexran_log.h"
#include "rt_controller_common.h"

flexran::core::rt::rt_task::rt_task(Policy pol, sched_priority priority,
    sched_time runtime, sched_time deadline, sched_time period) {
#ifdef LOWLATENCY
  set_scheduling_policy(pol, priority, runtime, deadline, period);
#else
  _unused(pol);
  _unused(priority);
  _unused(runtime);
  _unused(deadline);
  _unused(period);
#endif
}

#ifdef LOWLATENCY

void flexran::core::rt::rt_task::set_scheduling_policy(Policy pol,
    sched_priority priority, sched_time runtime, sched_time deadline, sched_time period)
{

  __u32 sched_policy;

  switch (pol) {
  case Policy::DEFAULT:
    sched_policy = SCHED_OTHER;
    priority = 0;
    runtime = 0;
    deadline = 0;
    period = 0;
    break;
  case Policy::FIFO:
    sched_policy = SCHED_FIFO;
    break;
  case Policy::DEADLINE:
    sched_policy = SCHED_DEADLINE;
    break;
  case Policy::RR:
    sched_policy = SCHED_RR;
    break;
  default:
    LOG4CXX_ERROR(flog::core, "unsupported policy " << static_cast<int>(pol)
        << ", using SCHED_RR instead");
    sched_policy = SCHED_RR;
  }

  sched_priority min = sched_get_priority_min(sched_policy);
  sched_priority max = sched_get_priority_max(sched_policy);
  if (priority < min) {
    LOG4CXX_ERROR(flog::core, "priority " << priority
        << " too low for policy, setting to min " << min);
    priority = min;
  }
  if (priority > max) {
    LOG4CXX_ERROR(flog::core, "priority " << priority
        << " too high for policy, setting to max " << max);
    priority = max;
  }
  
  attr_.size = sizeof(attr_);
  attr_.sched_policy = sched_policy;
  attr_.sched_flags = 0;
  attr_.sched_nice = 0;

  attr_.sched_priority = priority;

  attr_.sched_runtime  = runtime;
  attr_.sched_deadline = deadline;
  attr_.sched_period   = period;
}

#endif

void flexran::core::rt::rt_task::execute_task() {
#ifdef LOWLATENCY
  
  if (sched_setattr(0, &attr_, 0) < 0 ) {
    LOG4CXX_ERROR(flog::core, "sched_setattr failed: "
      << std::strerror(errno));
    LOG4CXX_INFO(flog::core, "Run with privileged rights or "
      << "consider compiling without low latency support.");
    return;
  }

#endif

  run();
}

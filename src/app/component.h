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

/*! \file    component.h
 *  \brief   application base class, defines structure
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <boost/thread/barrier.hpp>

#include "rt_task.h"
#include "rib.h"
#include "requests_manager.h"

namespace flexran {

  namespace app {

    class component : public core::rt::rt_task {
    public:

    component(rib::Rib& rib, const core::requests_manager&rm,
        Policy pol = Policy::RR, sched_priority priority = 20,
        sched_time runtime = 0, sched_time deadline = 0, sched_time period = 0)
      : rt_task(pol, priority, runtime, deadline, period),
        rib_(rib), req_manager_(rm) {}

      //! the barrier is for synchronization purposes. The task manager uses
      //  this to inform apps when they can run.
      void set_app_sync_barrier(std::shared_ptr<boost::barrier> barrier) { app_sync_barrier_ = barrier; }
      //! this method is for synchronization purposes. The task manager uses
      //  this to inform apps that they should finish after the the next call
      //  to app_sync_barrier_.
      void inform_exit() { _exit_app = true; }

      virtual void run_app() = 0;

    protected:
      const rib::Rib& rib_;
      const core::requests_manager& req_manager_;
      std::shared_ptr<boost::barrier> app_sync_barrier_;
      bool _exit_app = false;
  
    private:
      
      void run() { run_app(); }
      
    };

  }

}

#endif

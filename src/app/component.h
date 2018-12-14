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

#include "rt_task.h"
#include "rib.h"
#include "requests_manager.h"
#include "flexran_log.h"
#include "subscription.h"

namespace flexran {

  namespace app {

    class component : public core::rt::rt_task {
    public:

    component(const rib::Rib& rib, const core::requests_manager& rm,
        event::subscription& sub, Policy pol = Policy::RR,
        sched_priority priority = 20, sched_time runtime = 0,
        sched_time deadline = 0, sched_time period = 0)
      : rt_task(pol, priority, runtime, deadline, period),
        rib_(rib), req_manager_(rm), event_sub_(sub) {}

      //! this method is for synchronization purposes. It can be used to inform
      //  apps that they should finish after the the next call to run_app()
      void inform_exit() { _exit_app = true; }

      //! this method can be overridden in a subclass to implement how this
      //  function should be run in its own thread. In this case, the inherited
      //  rt_task method execute_task() would trigger this function together
      //  with the right scheduling policy has as set in the constructor.
      virtual void run_app() { LOG4CXX_ERROR(flog::app, "run_app() not implemented"); }

    protected:
      const rib::Rib& rib_;
      const core::requests_manager& req_manager_;
      event::subscription& event_sub_;
      bool _exit_app = false;
  
    private:
      
      void run() { run_app(); }
      
    };

  }

}

#endif

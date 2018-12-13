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

/*! \file    task_manager.h
 *  \brief   handles the tasks within the controller
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include "rt_task.h"
#include "rib_updater.h"
#include "rt_wrapper.h"
#include "component.h"
#include "subscription.h"

#include <linux/types.h>
#include <vector>
#include <memory>

#include <sys/timerfd.h>

namespace flexran {

  namespace core {

    class task_manager : public rt::rt_task {
    public:

      task_manager(flexran::rib::rib_updater& r_updater, flexran::event::subscription& ev);

      void manage_rt_tasks();

    private:
      
      void run();
  
      void wait_for_cycle();

#ifdef PROFILE
      static void profiler_wb_thread(std::unique_ptr<std::stringstream> ss, size_t num_app);
#endif
      
      flexran::rib::rib_updater& r_updater_;
      flexran::event::subscription& event_sub_;

      int sfd;

    };
  }
}

#endif

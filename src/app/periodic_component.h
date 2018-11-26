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

/*! \file    periodic_component.h
 *  \brief   base class of all apps: provides implementation of synchronization
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef PERIODIC_COMPONENT_H_
#define PERIODIC_COMPONENT_H_

#include "component.h"

namespace flexran {

  namespace app {

    class periodic_component : public component {

    public:
      
    periodic_component(rib::Rib& rib, const core::requests_manager& rm)
      : component(rib, rm) {}

    periodic_component(rib::Rib& rib, const core::requests_manager& rm,
        Policy pol, sched_priority priority, sched_time runtime,
        sched_time deadline, sched_time period)
      : component(rib, rm, pol, priority, runtime, deadline, period) {}

      void run_app() {}
      virtual void periodic_task() = 0;
      
    };

  }

}

#endif

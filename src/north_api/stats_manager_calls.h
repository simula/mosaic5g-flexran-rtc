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

/*! \file    stats_manager_calls.h
 *  \brief   NB API for statistics information
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef STATS_MANAGER_CALLS_H_
#define STATS_MANAGER_CALLS_H_

#include <string>
#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "stats_manager.h"

namespace flexran {

  namespace north_api {

    class stats_manager_calls : public app_calls {

    public:

      stats_manager_calls(std::shared_ptr<flexran::app::stats::stats_manager> stats)
      : stats_app(stats)
      { }

      void register_calls(Pistache::Rest::Description& desc);

      void obtain_stats(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      void obtain_json_stats(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void obtain_json_stats_enb(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void obtain_json_stats_ue(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void get_stats_req(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void set_stats_req(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    private:
      std::shared_ptr<flexran::app::stats::stats_manager> stats_app;

    };
  }
}


#endif /* FLEXIBLE_SCHED_CALLS_H_ */

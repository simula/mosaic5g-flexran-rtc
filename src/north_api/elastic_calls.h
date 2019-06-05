/*
 * Copyright 2016-2019 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    elastic_calls.h
 *  \brief   NB API for elastic search app
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef _ELASTIC_CALLS_H_
#define _ELASTIC_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "elastic_search.h"

namespace flexran {
  namespace north_api {
    class elastic_calls : public app_calls {
    public:
      elastic_calls(std::shared_ptr<flexran::app::log::elastic_search> elastic)
        : elastic_app(elastic)
      {}

      void register_calls(Pistache::Rest::Description& desc);

      void status(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void add_endpoint(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void remove_endpoint(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void set_freq_stats(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void set_freq_config(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void set_batch_stats_size(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void set_batch_config_size(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void enable(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);
      void disable(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);

    private:
      std::shared_ptr<flexran::app::log::elastic_search> elastic_app;
    };
  }
}

#endif /* _ELASTIC_CALLS_H_ */

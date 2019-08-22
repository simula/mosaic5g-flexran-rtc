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

/*! \file    aqm_calls.h
 *  \brief   NB API for AQM management
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef AQM_CALLS_H_
#define AQM_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "aqm_management.h"

namespace flexran {

  namespace north_api {

    class aqm_calls : public app_calls {

    public:

      aqm_calls(std::shared_ptr<flexran::app::management::aqm_management> aqm)
        : aqm_app(aqm)
      { }

      void register_calls(Pistache::Rest::Description& desc);

    private:
      void apply_aqm_config(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void apply_aqm_config_rnti(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      std::shared_ptr<flexran::app::management::aqm_management> aqm_app;

    };
  }
}


#endif /* AQM_CALLS_H_ */

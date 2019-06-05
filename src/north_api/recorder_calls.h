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

/*! \file    recorder_calls.h
 *  \brief   NB API for recorder app
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef _RECORDER_CALLS_H_
#define _RECORDER_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "recorder.h"

namespace flexran {

  namespace north_api {

    class recorder_calls : public app_calls {

    public:

      recorder_calls(std::shared_ptr<flexran::app::log::recorder> json)
	: json_app(json)
      {}

      void register_calls(Pistache::Rest::Description& desc);

      void start_meas(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);

      void obtain_json_stats(const Pistache::Rest::Request& request,
          Pistache::Http::ResponseWriter response);


    private:

      std::shared_ptr<flexran::app::log::recorder> json_app;

    };
  }
}

#endif /* _RECORDER_CALLS_H_ */

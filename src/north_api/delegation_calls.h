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

/*! \file    delegation_calls.h
 *  \brief   NB API for delegation manager
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef DELEGATION_CALLS_H_
#define DELEGATION_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "delegation_manager.h"

namespace flexran {

  namespace north_api {

    class delegation_calls : public app_calls {

    public:

      delegation_calls(std::shared_ptr<flexran::app::management::delegation_manager> delg)
        : delegation_app(delg)
      { }

      void register_calls(Pistache::Rest::Description& desc);

    private:
      void push_object(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter response);
      void load_object(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter response);

      std::shared_ptr<flexran::app::management::delegation_manager> delegation_app;
    };
  }
}

#endif /* DELEGATION_CALLS_H_ */

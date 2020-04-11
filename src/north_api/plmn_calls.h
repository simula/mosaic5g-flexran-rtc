/*
 * Copyright 2016-2020 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    plmn_calls.h
 *  \brief   NB API for PLMN and MME control
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef PLMN_CALLS_H_
#define PLMN_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "plmn_management.h"

namespace flexran {

  namespace north_api {

    class plmn_calls : public app_calls {

    public:

      plmn_calls(std::shared_ptr<flexran::app::management::plmn_management> plmn)
        : plmn_app(plmn)
      { }

      void register_calls(Pistache::Rest::Description& desc);

      void remove_mme(const Pistache::Rest::Request& request,
                      Pistache::Http::ResponseWriter response);

      void add_mme(const Pistache::Rest::Request& request,
                   Pistache::Http::ResponseWriter response);

      void change_plmn(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter response);

    private:

      std::shared_ptr<flexran::app::management::plmn_management> plmn_app;

    };
  }
}


#endif /* PLMN_CALLS_H_ */

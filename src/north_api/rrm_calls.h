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

/*! \file    rrm_calls.h
 *  \brief   NB API for RRM policies
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef RRM_CALLS_H_
#define RRM_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "rrm_management.h"

namespace flexran {

  namespace north_api {

    class rrm_calls : public app_calls {

    public:

      rrm_calls(std::shared_ptr<flexran::app::management::rrm_management> rrm)
        : rrm_app(rrm)
      { }
      
      void register_calls(Pistache::Rest::Description& desc);

      //void change_scheduler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void yaml_compat(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      void apply_slice_config(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void apply_slice_config_short(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      void remove_slice_config(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void remove_slice_config_short(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      void change_ue_slice_assoc(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void change_ue_slice_assoc_short(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      void instantiate_vnetwork(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void remove_vnetwork(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void associate_ue_vnetwork(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void remove_ue_list_vnetwork(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
      void remove_ue_vnetwork(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

      void cell_reconfiguration(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    private:

      std::shared_ptr<flexran::app::management::rrm_management> rrm_app;

    };
  }
}


#endif /* RRM_CALLS_H_ */

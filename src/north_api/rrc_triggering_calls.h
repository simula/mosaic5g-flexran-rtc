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

/*! \file    rrc_triggering_calls.h
 *  \brief   NB API for RRC triggering app
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef RRC_TRIGGERING_CALLS_H_
#define RRC_TRIGGERING_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "rrc_triggering.h"

namespace flexran {

  namespace north_api {

    class rrc_triggering_calls : public app_calls {

    public:

      rrc_triggering_calls(std::shared_ptr<flexran::app::rrc::rrc_triggering> flex_trigger)
	: rrc_trigger(flex_trigger)
      { }
      
      void register_calls(Pistache::Rest::Description& desc);

      void rrc_reconf(const Pistache::Rest::Request& request,
                      Pistache::Http::ResponseWriter response);
      void rrc_ho(const Pistache::Rest::Request& request,
                  Pistache::Http::ResponseWriter response);
      void rrc_x2_ho_net_control(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response);

    private:

      std::shared_ptr<flexran::app::rrc::rrc_triggering> rrc_trigger;

    };
  }
}


#endif /* RRC_TRIGGERING_CALLS_H_ */

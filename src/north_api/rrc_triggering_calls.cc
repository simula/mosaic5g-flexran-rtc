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

/*! \file    rrc_triggering_calls.cc
 *  \brief   NB API for RRC triggering app
 *  \authors Shahab SHARIAT BAGHERI
 *  \company Eurecom
 *  \email   shahab.shariat@eurecom.fr
 */

#include <pistache/http.h>

#include "rrc_triggering_calls.h"

void flexran::north_api::rrc_triggering_calls::register_calls(Pistache::Rest::Router& router) {

  Pistache::Rest::Routes::Post(router, "/rrc_trigger/:trigger_type", Pistache::Rest::Routes::bind(&flexran::north_api::rrc_triggering_calls::change_rrc, this));  
}

void flexran::north_api::rrc_triggering_calls::change_rrc(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto Trigger_type = request.param(":trigger_type").as<std::string>();
  
  if (Trigger_type.compare("one_shot") == 0) { 
    rrc_trigger->enable_rrc_triggering(Trigger_type);
    response.send(Pistache::Http::Code::Ok, "Trigger one shot");
  } else if (Trigger_type.compare("periodical") == 0) { //Remote scheduler 
    rrc_trigger->enable_rrc_triggering(Trigger_type);
    response.send(Pistache::Http::Code::Ok, "Trigger periodical");
   } else if (Trigger_type.compare("event_driven") == 0) { //Remote scheduler 
    rrc_trigger->enable_rrc_triggering(Trigger_type);
    response.send(Pistache::Http::Code::Ok, "Trigger event_driven"); 
  } else { // Scheduler type not supported
    response.send(Pistache::Http::Code::Not_Found, "Trigger type does not exist");
  }
  
}

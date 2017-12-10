/* The MIT License (MIT)

   Copyright (c) 2017 shahab SHARIAT BAGHERI

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
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
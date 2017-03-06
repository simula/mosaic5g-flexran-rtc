/* The MIT License (MIT)

   Copyright (c) 2017 

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

void flexran::north_api::flexible_rrc_calls::register_calls(Net::Rest::Router& router) {

  Net::Rest::Routes::Post(router, "/rrc_trigger/:trigger_type", Net::Rest::Routes::bind(&flexran::north_api::flexible_rrc_calls::change_rrc, this));
  
}

void flexran::north_api::flexible_rrc_calls::change_rrc(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {

  auto Trigger_type = request.param(":trigger_type").as<int>();
  
  if (Trigger_type == 0) { // Local scheduler
    rrc_trigger->enable_central_scheduling();
    response.send(Net::Http::Code::Ok, "Trigger 0");
  } else if (Trigger_type == 1) { //Remote scheduler 
    rrc_trigger->enable_central_scheduling();
    response.send(Net::Http::Code::Ok, "Trigger 1");
  } else { // Scheduler type not supported
    response.send(Net::Http::Code::Not_Found, "Trigger type does not exist");
  }
  
}
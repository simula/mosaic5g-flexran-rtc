/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

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
#include <string>

#include "enb_sched_policy_calls.h"

void flexran::north_api::enb_sched_policy_calls::register_calls(Net::Rest::Router& router) {

  // the URL example : curl -x  POST http://localhost:9999/dl_sched/0 
  // : means that this is variable 
  // give the policyname, policyname will be a variable
  // for policyname: curl -x  POST http://localhost:9999/dl_sched/policy.yaml 
  Net::Rest::Routes::Post(router, "/dl_policy/:policyname", Net::Rest::Routes::bind(&flexran::north_api::enb_sched_policy_calls::apply_policy, this));
  // Net::Rest::Routes::Post(router, "/dl_policy/:rb_share", Net::Rest::Routes::bind(&flexran::north_api::enb_sched_policy_calls::set_policy, this));
  
}

void flexran::north_api::enb_sched_policy_calls::apply_policy(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {

  auto policy_name = request.param(":policyname").as<std::string>();
  std::string resp;
  if (policy_name != "") { // Local scheduler
    sched_app->apply_policy(policy_name);
    response.send(Net::Http::Code::Ok, "Set the policy to the agent");
  } else { // Scheduler policy not set
    response.send(Net::Http::Code::Not_Found, "Policy not set");
  }
  
}

/*
void flexran::north_api::enb_sched_policy_calls::set_policy(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {

  auto rb_share = request.param(":rb_share").as<int>();
  std::string resp;
  
  if (rb_share >= 0 && rb_share <=1) { // Local scheduler
    sched_app->set_policy(rb_share);
    response.send(Net::Http::Code::Ok, "Set the RB share for different slice");
 } else { // Scheduler type not supported
    response.send(Net::Http::Code::Not_Found, "rb_share value out of range");
  }
  
}
*/

/*
void flexran::north_api::enb_sched_policy_calls::change_scheduler(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {

  auto sched_type = request.param(":sched_type").as<int>();
  
  if (sched_type == 0) { // Local scheduler
    sched_app->enable_central_scheduling(false);
    response.send(Net::Http::Code::Ok, "Loaded Local Scheduler");
  } else if (sched_type == 1) { //Remote scheduler 
    sched_app->enable_central_scheduling(true);
    response.send(Net::Http::Code::Ok, "Loaded Remote Scheduler");
  } else { // Scheduler type not supported
    response.send(Net::Http::Code::Not_Found, "Scheduler type does not exist");
  }
  
}

*/

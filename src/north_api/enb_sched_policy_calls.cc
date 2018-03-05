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

void flexran::north_api::enb_sched_policy_calls::register_calls(Pistache::Rest::Router& router) {

  // the URL example : curl -x  POST http://localhost:9999/rrm/0 
  // : means that this is variable 
  // give the policyname, policyname will be a variable
  // for policyname: curl -x  POST http://localhost:9999/rrm/policy.yaml 
  Pistache::Rest::Routes::Post(router, "/rrm/:policyname", Pistache::Rest::Routes::bind(&flexran::north_api::enb_sched_policy_calls::apply_policy, this));
  // Pistache::Rest::Routes::Post(router, "/rrm/:rb_share", Pistache::Rest::Routes::bind(&flexran::north_api::enb_sched_policy_calls::set_policy, this));

   /**
     * @api {post} /rrm/:policyname Post the RAN policy command by function.
     * @apiName ApplyPolicy
     * @apiGroup user/slice/BS policies
     * @apiDescription This API endpoint post a policy to the underlying BS. It can be used to create, update, and delete a slice on the top of BS.
     * @apiVersion v0.1.0
     * @apiPermission None
     * @apiParam {string} policy.yaml RAN policy file
     * @apiExample Example usage:
     *     curl -X POST http://127.0.0.1:9999/rrm/policy.yaml
     *
     * A complete example of a policy.yaml file:
     *
     * BS1:
     *  node_function: "eNodeB_3GPPP"
     *  eutra_band: val1
     *  downlink_frequency: val
     *  uplink_frequency_offset: val
     *  N_RB_DL: val
     *
     * mac:
     *  - dl_scheduler:
     *     behaviour: <callback>
     *     parameters:
     *       n_active_slices: val
     *       slice_maxmcs: [val1, val2, val3, val4]
     *       slice_percentage: [val1, val2, val3, val4]
     *       slice_rb_map: [val1, val2, val3, val4]
     * - ul_scheduler:
     *    behaviour: <callback>
     *    parameters:
     *       n_active_slices_uplink: val
     *       slice_maxmcs_uplink: [val1, val2, val3, val4]
     *       slice_percentage_uplink: [val1, val2, val3, val4]
     *       slice_rb_map_uplink: [val1, val2, val3, val4]
     *
     * @apiSuccessExample Success-Response:
     *     HTTP/1.1 200 OK
     *
     * @apiError PolicyFileNotFound The policy file not found.
     *
     * @apiErrorExample Error-Response:
     *     HTTP/1.1 404 Not Found
     *     {
     *       "error": "policy file not found"
     *     }
     */


  /*
 * rrc:
     * - trigger_measurment : val
     * - x2_ho:
     *   parameters:
     *     ttt_ms 		: val
     *     hys 	  	: val
     *     ofn 	  	: val
     *     ocn 	  	: val
     *     ofs 	  	: val
     *     ocs 	  	: val
     *     off 	  	: val
     *     filter_coeff_rsrp 	: val
     *     filter_coeff_rsrq 	: val
     */
}

void flexran::north_api::enb_sched_policy_calls::apply_policy(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto policy_name = request.param(":policyname").as<std::string>();
  std::string resp;
  if (policy_name != "") { // Local scheduler
    sched_app->apply_policy(policy_name);
    response.send(Pistache::Http::Code::Ok, "Set the policy to the agent\n");
  } else { // Scheduler policy not set
    response.send(Pistache::Http::Code::Not_Found, "Policy not set\n");
  }
  
}

/*
void flexran::north_api::enb_sched_policy_calls::set_policy(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto rb_share = request.param(":rb_share").as<int>();
  std::string resp;
  
  if (rb_share >= 0 && rb_share <=1) { // Local scheduler
    sched_app->set_policy(rb_share);
    response.send(Pistache::Http::Code::Ok, "Set the RB share for different slice");
 } else { // Scheduler type not supported
    response.send(Pistache::Http::Code::Not_Found, "rb_share value out of range");
  }
  
}
*/

/*
void flexran::north_api::enb_sched_policy_calls::change_scheduler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto sched_type = request.param(":sched_type").as<int>();
  
  if (sched_type == 0) { // Local scheduler
    sched_app->enable_central_scheduling(false);
    response.send(Pistache::Http::Code::Ok, "Loaded Local Scheduler");
  } else if (sched_type == 1) { //Remote scheduler 
    sched_app->enable_central_scheduling(true);
    response.send(Pistache::Http::Code::Ok, "Loaded Remote Scheduler");
  } else { // Scheduler type not supported
    response.send(Pistache::Http::Code::Not_Found, "Scheduler type does not exist");
  }
  
}

*/

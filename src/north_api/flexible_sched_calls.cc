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

#include "flexible_sched_calls.h"

void flexran::north_api::flexible_sched_calls::register_calls(Pistache::Rest::Router& router)
{
  /**
   * @api {post} /dl_sched/:sched_type Set scheduler type
   * @apiName DlSchedType
   * @apiGroup user/slice/BS policies
   *
   * @apiDeprecated This method is for internal tests and should not be used.
   * It might be dysfunctional and be removed in the future.
   *
   * @apiDescription Used to set the DL scheduler policy.
   *
   * @apiParam {number} sched_type The DL scheduler policy. 0 for local, 1 for
   * remote scheduler.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   */
  Pistache::Rest::Routes::Post(router, "/dl_sched/:sched_type", Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::change_scheduler, this));

  /**
   * @api {post} /rrm_config/ Post a user-defined RAN policy command
   * @apiName ApplyPolicyString
   * @apiGroup user/slice/BS policies
   *
   * @apiDescription This API endpoint posts a policy to the underlying BS.
   * It can be used to create, update, and delete a slice on top of a BS.
   */
  Pistache::Rest::Routes::Post(router, "/slice/enb/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::apply_slice_config, this));

  /**
   * @api {post} /rrm_config/ Post a user-defined RAN policy command, no body
   * @apiName ApplyPolicyString
   * @apiGroup user/slice/BS policies
   *
   * @apiDescription This API endpoint posts a policy to the underlying BS.
   * It can be used to create, update, and delete a slice on top of a BS.
   */
  Pistache::Rest::Routes::Post(router, "/slice/enb/:id/slice/:slice_id",
      Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::apply_slice_config_short, this));

  Pistache::Rest::Routes::Delete(router, "/slice/enb/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::remove_slice_config, this));

  Pistache::Rest::Routes::Delete(router, "/slice/enb/:id/slice/:slice_id",
      Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::remove_slice_config_short, this));

  /**
   * @api {post} /ue_slice_assoc/ Post a user-defined UE-to-slice association
   * @apiName ApplyPolicyString
   * @apiGroup user/slice/BS policies
   *
   * @apiDescription This API endpoint posts a policy to the underlying BS.
   * It can be used to change the association of a UE to a slice. Currently,
   * only the RNTI is supported.
   */
  Pistache::Rest::Routes::Post(router, "/ue_slice_assoc/enb/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::change_ue_slice_assoc, this));
}

void flexran::north_api::flexible_sched_calls::change_scheduler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

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

void flexran::north_api::flexible_sched_calls::apply_slice_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int agent_id = request.hasParam(":id") ?
      sched_app->parse_enb_agent_id(request.param(":id").as<std::string>()) :
      sched_app->get_last_agent();
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!sched_app->apply_slice_config_policy(agent_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::flexible_sched_calls::apply_slice_config_short(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int agent_id = sched_app->parse_enb_agent_id(request.param(":id").as<std::string>());
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  int slice_id = request.param(":slice_id").as<int>();
  std::string policy;
  policy  = "{\"dl\":[{id:" + std::to_string(slice_id);
  policy += "}],\"ul\":[{id:" + std::to_string(slice_id);
  policy += "}]}";

  std::string error_reason;
  if (!sched_app->apply_slice_config_policy(agent_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::flexible_sched_calls::remove_slice_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int agent_id = request.hasParam(":id") ?
      sched_app->parse_enb_agent_id(request.param(":id").as<std::string>()) :
      sched_app->get_last_agent();
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!sched_app->remove_slice(agent_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::flexible_sched_calls::remove_slice_config_short(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int agent_id = sched_app->parse_enb_agent_id(request.param(":id").as<std::string>());
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  int slice_id = request.param(":slice_id").as<int>();
  std::string policy;
  policy  = "{\"dl\":[{id:" + std::to_string(slice_id);
  policy += ",percentage:0}],\"ul\":[{id:" + std::to_string(slice_id);
  policy += ",percentage:0}]}";

  std::string error_reason;
  if (!sched_app->remove_slice(agent_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::flexible_sched_calls::change_ue_slice_assoc(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int agent_id = request.hasParam(":id") ?
      sched_app->parse_enb_agent_id(request.param(":id").as<std::string>()) :
      sched_app->get_last_agent();
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!sched_app->change_ue_slice_association(agent_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

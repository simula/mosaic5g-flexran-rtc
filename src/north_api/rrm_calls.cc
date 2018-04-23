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

#include "flexran_log.h"
#include "rrm_calls.h"

void flexran::north_api::rrm_calls::register_calls(Pistache::Rest::Router& router)
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
  Pistache::Rest::Routes::Post(router, "/dl_sched/:sched_type", Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::change_scheduler, this));

  /**
   * @api {post} /slice/enb/:id? Post a slice configuration
   * @apiName ApplySliceConfiguration
   * @apiGroup SliceConfiguration
   * @apiParam {Number} [id=-1] The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   *
   * @apiDescription This API endpoint posts a new slice configuration to an
   * underlying agent, specified as a JSON file with the format of the
   * `sliceConfig` as contained in the `cellConfig` of an agent configuration.
   * It can be used to create arbitrary slices with an arbitrary ID in the
   * range [1,255] or to change slices in the range [0,255]. In the request, a
   * slice ID must present. All other values will be copied from slice ID 0
   * The caller should take care that the sum of slice percentages (i.e. of all
   * present and added slices) should not exceed 100, as this is not catched at
   * the controller but enforced at the MAC scheduler. The `stats` call should
   * always be used after a call and sufficient time to verify the actions have
   * been taken.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/slice/enb/-1 --data-binary "@file"
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "dl": [
   *        {
   *          "id": 0,
   *          "percentage": 25,
   *          "maxmcs": 28
   *        },
   *        {
   *          "id": 3,
   *          "percentage": 25,
   *          "maxmcs": 28
   *        }
   *      ],
   *      "ul": [
   *        {
   *          "id": 0,
   *          "percentage": 25,
   *          "maxmcs": 20
   *        },
   *        {
   *          "id": 3,
   *          "percentage": 25,
   *          "maxmcs": 20
   *        }
   *      ]
   *    }
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find agent" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "missing slice ID" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Protobuf parser error" }
   */
  Pistache::Rest::Routes::Post(router, "/slice/enb/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::apply_slice_config, this));

  /**
   * @api {post} /slice/enb/:id? Create a new pair of slices (short version)
   * @apiName ApplySliceConfigurationShort
   * @apiGroup SliceConfiguration
   * @apiParam {Number} id The ID of the agent for which to change the slice
   * configuration. This can be one of the following: -1 (last added agent),
   * the eNB ID (in hex or decimal) or the internal agent ID which can be
   g obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam {Number} slice_id The ID of the slices (UL and DL) to be created
   * in the range [1,255].
   *
   * @apiDescription This API endpoint creates a new pair of slices copying the
   * values of the slice 0.  It can be used to create arbitrary slices with an
   * arbitrary ID in the range [1,255]. Please note that if slice 0 has already
   * more than 50 percent, this call will fail (since the percentage value is
   * copied, too). The caller should take care that the sum of slice
   * percentages (i.e. of all present and added slices) should not exceed 100,
   * as this is not catched at the controller but enforced at the MAC
   * scheduler. The `stats` call should always be used after a call and
   * sufficient time to verify the actions have been taken.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/slice/enb/-1/slice/13
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Missing or wrong parameters, reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find agent" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "invalid slice ID" }
   */
  Pistache::Rest::Routes::Post(router, "/slice/enb/:id/slice/:slice_id",
      Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::apply_slice_config_short, this));

  /**
   * @api {delete} /slice/enb/:id? Delete slices
   * @apiName DeleteSlice
   * @apiGroup SliceConfiguration
   * @apiParam {Number} [id=-1] The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   *
   * @apiDescription This API endpoint deletes slices as specified in the JSON
   * data in the body specified as a JSON file with the format of the
   * `sliceConfig` as contained in the `cellConfig` of a agent configuration
   * for a given agent.  In particular, a valid slice ID must present and the
   * percentage value must be zero. Slice 0 can not be removed.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X DELETE http://127.0.0.1:9999/slice/enb/-1 --data-binary "@file"
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "dl": [
   *        {
   *          "id": 3,
   *          "percentage": 0
   *        }
   *      ],
   *      "ul": [
   *        {
   *          "id": 3,
   *          "percentage": 0
   *        }
   *      ]
   *    }
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find agent" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "missing slice ID" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Protobuf parser error" }
   */
  Pistache::Rest::Routes::Delete(router, "/slice/enb/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::remove_slice_config, this));

  /**
   * @api {delete} /slice/enb/:id/slice/:slice_id Delete slices (short version)
   * @apiName DeleteSliceShort
   * @apiGroup SliceConfiguration
   * @apiParam {Number} id The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam {Number} slice_id The ID of the slices (UL and DL) to be created
   * in the range [1,255].
   *
   * @apiDescription This API endpoint deletes the UL and DL slices with ID
   * `slice_id` (only one ID can be provided for both slices) for a given agent.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X DELETE http://127.0.0.1:9999/slice/enb/-1/slice_id/3
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find agent" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find DL slice ID" }
   */
  Pistache::Rest::Routes::Delete(router, "/slice/enb/:id/slice/:slice_id",
      Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::remove_slice_config_short, this));

  /**
   * @api {post} /ue_slice_assoc/enb/:id? Change the UE-slice association
   * @apiName ChangeUeSliceAssociation
   * @apiGroup SliceConfiguration
   * @apiParam {Number} [id=-1] The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   *
   * @apiDescription This API endpoint changes the association of a UE in an
   * underlying agent, specified as a JSON file with the format of the
   * `ueConfig` as contained in the agent configuration.  It can be used to
   * changed the association of UEs using their current RNTI or IMSI. In the
   * request, a slice ID and RNTI or IMSI must be present. The `stats` call
   * should always be used after a call and sufficient time to verify the
   * actions have been taken.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/ue_slice_assoc/enb/-1 --data-binary "@file"
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "ueConfig": [
   *        {
   *          "imsi": 208940100001115,
   *          "dlSliceId": 3,
   *          "ulSliceId": 3
   *        }
   *      ]
   *    }

   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find agent" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "no such slice" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Protobuf parser error" }
   */
  Pistache::Rest::Routes::Post(router, "/ue_slice_assoc/enb/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::change_ue_slice_assoc, this));

  /**
   * @api {post} /yaml/:id? Send arbitrary YAML to the agent
   * @apiName YamlCompat
   * @apiGroup user/slice/BS policies
   *
   * @apiDeprecated This method is for internal tests and should not be used.
   * It might be dysfunctional or make the agent break and be removed in the
   * future.
   *
   * @apiDescription Send arbitrary YAML files to the indicated agent.
   *
   * @apiParam {Number} [id=-1] The ID of the agent to which the file should be
   * sent. This can be one of the following: -1 (last added agent), the eNB ID
   * (in hex or decimal) or the internal agent ID which can be obtained through
   * a `stats` call. Numbers smaller than 1000 are parsed as the agent ID.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   */
  Pistache::Rest::Routes::Post(router, "/yaml/:id?",
      Pistache::Rest::Routes::bind(&flexran::north_api::rrm_calls::yaml_compat, this));
}

void flexran::north_api::rrm_calls::change_scheduler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

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

void flexran::north_api::rrm_calls::apply_slice_config(
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

void flexran::north_api::rrm_calls::apply_slice_config_short(
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

void flexran::north_api::rrm_calls::remove_slice_config(
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

void flexran::north_api::rrm_calls::remove_slice_config_short(
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

void flexran::north_api::rrm_calls::change_ue_slice_assoc(
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

void flexran::north_api::rrm_calls::yaml_compat(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int agent_id = request.hasParam(":id") ?
      sched_app->parse_enb_agent_id(request.param(":id").as<std::string>()) :
      sched_app->get_last_agent();
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Not_Found, "Policy not set (no such agent)\n");
    return;
  }
  if (request.body().length() == 0) {
    response.send(Pistache::Http::Code::Not_Found, "Policy not set (body is empty)\n");
    return;
  }

  LOG4CXX_INFO(flog::app, "sending YAML request to agent " << agent_id
      << " (compat):\n" << request.body());
  sched_app->reconfigure_agent_string(agent_id, request.body());
  response.send(Pistache::Http::Code::Ok, "Set the policy to the agent\n");
}

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
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>

#include "rrc_triggering_calls.h"

void flexran::north_api::rrc_triggering_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto rrc_trigger = desc.path("/rrc");

  /**
   * @api {post} /rrc/reconf/enb/:id? Reconfigure RRC parameters
   * @apiName ReconfigureRrcParam
   * @apiGroup RRC Control
   *
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the affected BS. Can be:
   * -1 (last added BS), the BS ID (in hex or decimal).
   * @apiParam (JSON parameter) {Number} offsetFreqServing See TS 36.331
   * @apiParam (JSON parameter) {Number} offsetFreqNeighbouring See TS 36.331
   * @apiParam (JSON parameter) {Number} cellIndividualOffset See TS 36.331
   * @apiParam (JSON parameter) {Number} filterCoefficientRsrp See TS 36.331
   * @apiParam (JSON parameter) {Number} filterCoefficientRsrq See TS 36.331
   * @apiParam (JSON parameter) {Object="periodical","a1","a2","a3","a4","a5"} event
   * @apiParam (periodical event parameter) {Number} max_report_cells The number of
   * cells to report. See TS 36.331 5.5.5
   * @apiParam (a1, a2, a4 event parameter) {Number} threshold_rsrp  See TS 36.331 5.5.4.2/5.5.4.3/5.5.4.5
   * @apiParam (a1, a2, a4 event parameter) {Number} hysteresis See TS 36.331 5.5.4.2/5.5.4.3/5.5.4.5
   * @apiParam (a1, a2, a4 event parameter) {Number} time_to_trigger See TS 36.331 5.5.4.2/5.5.4.3/5.5.4.5
   * @apiParam (a1, a2, a4 event parameter) {Number} max_report_cells See TS 36.331 5.5.4.2/5.5.4.3/5.5.4.5
   * @apiParam (a3 event parameter) {Number} a3_offset See TS 36.331 5.5.4.4
   * @apiParam (a3 event parameter) {Number} report_on_leave See TS 36.331 5.5.4.4
   * @apiParam (a3 event parameter) {Number} hysteresis See TS 36.331 5.5.4.4
   * @apiParam (a3 event parameter) {Number} time_to_trigger See TS 36.331 5.5.4.4
   * @apiParam (a3 event parameter) {Number} max_report_cells See TS 36.331 5.5.4.4
   * @apiParam (a5 event parameter) {Number} threshold_rsrp_1 See TS 36.331 5.5.4.6
   * @apiParam (a5 event parameter) {Number} threshold_rsrp_2 See TS 36.331 5.5.4.6
   * @apiParam (a5 event parameter) {Number} hysteresis See TS 36.331 5.5.4.6
   * @apiParam (a5 event parameter) {Number} time_to_trigger See TS 36.331 5.5.4.6
   * @apiParam (a5 event parameter) {Number} max_report_cells See TS 36.331 5.5.4.6
   *
   * @apiDescription Reconfigure RRC parameters in a given cell. For more
   * information, consult 3GPP TS 36.331. The structure of the JSON for this
   * call is the same as the `rrc_info` field in the UE configuration
   * section. Currently (06/06/19), the OAI agent only supports `a3` and
   * `periodic` event. Furthermore, this endpoint changes the configuration for
   * all UEs at once.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/rrc/reconf/enb/ --data-binary "@json-file"
   *
   * @apiParamExample {json} Request-Example:
   *  {
   *    "offsetFreqServing": "0",
   *    "offsetFreqNeighbouring": "0",
   *    "cellIndividualOffset": [
   *      "-24"
   *    ],
   *    "filterCoefficientRsrp": "5",
   *    "filterCoefficientRsrq": "5",
   *    "event": {
   *      "a3": {
   *        "a3Offset": "0",
   *        "reportOnLeave": 1,
   *        "hysteresis": "0",
   *        "timeToTrigger": "40",
   *        "maxReportCells": "2"
   *      }
   *    }
   *  }
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Missing or wrong parameters, reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find BS" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "ProtoBuf parser error" }
   */
  rrc_trigger.route(desc.post("/reconf/enb/:id?"), "Reconfigure RRC parameters")
             .bind(&flexran::north_api::rrc_triggering_calls::rrc_reconf, this);

  /**
   * @api {post} /rrc/ho/senb/:sid/ue/:ue/tenb/:tid Trigger HO
   * @apiName TriggerHO
   * @apiGroup RRC Control
   *
   * @apiParam (URL parameter) {Number} sid The ID of the source BS. Can be:
   * -1 (last added BS), the BS ID (in hex or decimal) or the physical Cell ID
   * which can obtained through a `stats` call.
   * @apiParam (URL parameter) {Number} ue The UE ID. Can be either the RNTI or
   * the IMSI (if known).
   * @apiParam (URL parameter) {Number} tid The ID of the target BS. Can be:
   * -1 (last added BS), the BS ID (in hex or decimal) or the physical Cell ID
   * which can obtained through a `stats` call.
   *
   * @apiDescription Triggers a network-initiated handover request. This sends
   * a command to the source BS to perform the handover of the given UE to the
   * target BS. The end-point checks that source BS and target BS exist (and
   * are non-equal) and that the UE is in the source BS. It does not check
   * whether the target BS is reachable for the UE!
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/rrc/ho/senb/1234567/ue/208950000000003/tenb/9876543
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Missing or wrong parameters, reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not source find BS" }
   */
  rrc_trigger.route(desc.post("/ho/senb/:sid/ue/:ue/tenb/:tid"),
                    "Trigger network-initiated handover")
             .bind(&flexran::north_api::rrc_triggering_calls::rrc_ho, this);

  /**
   * @api {post} /rrc/x2_ho_net_control/enb/:id/:bool Toggle HO NetControl State
   * @apiName ToggleHONetControl
   * @apiGroup RRC Control
   *
   * @apiParam (URL parameter) {Number} id The ID of the BS. Can be:
   * -1 (last added BS), the BS ID (in hex or decimal) or the agent ID
   * which can obtained through a `stats` call.
   * @apiParam (URL parameter) {Bool=0,1} bool Whether HO control is network-
   * or user-initiated.
   *
   * @apiDescription Toggles whether handover control is initiated through the
   * network or the user. If `true`, the network will perform handovers when
   * instructed to do so by the controller and any UE HO requests will be
   * ignored. If `false`, UE HO requests will be honored.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/rrc/x2_ho_net_control/enb/1234567/1
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Missing or wrong parameters, reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "can not find BS" }
   */
  rrc_trigger.route(desc.post("/x2_ho_net_control/enb/:id/:bool"),
                    "Enable/Disable X2 handover network control")
             .bind(&flexran::north_api::rrc_triggering_calls::rrc_x2_ho_net_control, this);
}

void flexran::north_api::rrc_triggering_calls::rrc_reconf(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs = "";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!rrc_trigger->rrc_reconf(bs, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::rrc_triggering_calls::rrc_ho(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string senb = request.param(":sid").as<std::string>();
  std::string ue   = request.param(":ue").as<std::string>();
  std::string tenb = request.param(":tid").as<std::string>();

  std::string error_reason;
  if (!rrc_trigger->rrc_ho(senb, ue, tenb, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::rrc_triggering_calls::rrc_x2_ho_net_control(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  bool x2_ho_net_control = request.param(":bool").as<bool>();
  std::string enb = request.param(":id").as<std::string>();
  std::string error_reason;
  if (!rrc_trigger->rrc_x2_ho_net_control(enb, x2_ho_net_control, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

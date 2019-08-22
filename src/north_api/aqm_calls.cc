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

/*! \file    aqm_calls.cc
 *  \brief   NB API for AQM management
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>

#include "flexran_log.h"
#include "aqm_calls.h"

void flexran::north_api::aqm_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto aqm_calls = desc.path("/aqm");

  ///**
  // * @api {post} /slice/enb/:id? Post a slice configuration
  // * @apiName ApplySliceConfiguration
  // * @apiGroup SliceConfiguration
  // *
  // * @apiParam (URL parameter) {Number} [id=-1] The ID of the agent for which
  // * to change the slice configuration. This can be one of the following: -1
  // * (last added agent), the eNB ID (in hex or decimal) or the internal agent
  // * ID which can be obtained through a `stats` call. Numbers smaller than 1000
  // * are parsed as the agent ID.
  // *
  // * @apiParam (JSON parameters) {Boolean} [intrasliceShareActive] The
  // * activation status of the intra-slice sharing phase. If active, RBs that
  // * are available after the first intra-slice allocation will be allocated to
  // * UEs belonging to the same slice that need it.
  // * @apiParam (JSON parameters) {Boolean} [intersliceShareActive] The
  // * activation status of the inter-slice multiplexing phase. If active, RBs
  // * that are available after the allocation of a slice, will be allocated to
  // * UEs belonging to the other slices that need it. Isolated slices are always
  // * ignored in this phase.
  // * @apiParam (JSON parameters) {Object[]} [dl] A list of DL slice
  // * configuration objects. For its parameters, see the parameters in the `JSON
  // * DL part parameters` table below.
  // * @apiParam (JSON parameters) {Object[]} [ul] A list of UL slice
  // * configuration objects. For its parameters, see the parameters in the `JSON
  // * UL part parameters` table below.
  // *
  // * @apiParam (JSON DL part parameters) {Number{0-255}} id The unique ID of
  // * the addressed DL slice.
  // * @apiParam (JSON DL part parameters) {String="xMBB","URLLC","mMTC","xMTC","Other"} [label]
  // * A descriptive label for this slice. It has currently no
  // * impact on the slice itself but might be extended to provide appropriate
  // * default values in the future.
  // * @apiParam (JSON DL part parameters) {Number{1-100}} [percentage] The
  // * number of resource blocks that this slice is allowed to use, as a fraction
  // * of the whole bandwidth. Please note that the sum of all slices is not
  // * allowed to exceed 100.
  // * @apiParam (JSON DL part parameters) {Boolean} [isolation] Whether this
  // * slice is isolated with regard to others. Refer to the
  // * `intersliceShareActive` to see the interaction with this parameter. If a
  // * slice is not isolated, unused RBs will be shared with other slices in the
  // * inter-slice multiplexing phase.
  // * @apiParam (JSON DL part parameters) {Number{0-20}} [priority] The priority
  // * of the slice when scheduling in the interslice multiplexing stage
  // * allocating in a greedy manner. Higher priority means preferential
  // * scheduling.
  // * @apiParam (JSON DL part parameters) {Number{0-24}} [positionLow] When
  // * positioning a slice in the frequency plane, this parameter marks the lower
  // * end (inclusive, i.e. `posL <= RB`).  Expressed in *RBG*. Must be lower
  // * than `positionHigh`. During the multiplexing phase, other RBs could be
  // * chosen, too. See `intersliceShareActive` for more information.
  // * @apiParam (JSON DL part parameters) {Number{1-25}} [positionHigh] When
  // * positioning a slice in the frequency plane, this parameter marks the high
  // * end (inclusive, i.e. `RB <= posH`).  Expressed in *RBG*. Must be higher
  // * than `positionLow`. During the multiplexing phase, other RBs could be
  // * chosen, too. See `intersliceShareActive` for more information.
  // * @apiParam (JSON DL part parameters) {Number{0-28}} [maxmcs] The maximum
  // * MCS that this slice is allowed to use.
  // * @apiParam (JSON DL part parameters) {String[]="CR_ROUND","CR_SRB12","CR_HOL","CR_LC","CR_CQI","CR_LCP"} [sorting]
  // * The policy by which users within a slice will be sorted before scheduling.
  // * * `"CR_ROUND"`: Highest HARQ first.
  // * * `"CR_SRB12"`: Highest SRB1+2 first.
  // * * `"CR_HOL"`:   Highest HOL first.
  // * * `"CR_LC"`:    Highest RLC buffer first.
  // * * `"CR_CQI"`:   Highest CQI first.
  // * * `"CR_LCP"`:   Highest LC priority first.
  // * @apiParam (JSON DL part parameters) {String="POL_FAIR","POL_GREEDY"} [accounting]
  // * The algorithm used in the accounting phase, i.e. when allocating the
  // * resources to the UEs after having sorted them with respect to the
  // * `sorting` parameter.
  // * @apiParam (JSON DL part parameters) {String="schedule_ue_spec"} [schedulerName]
  // * The name of the scheduler to be loaded. Can not be changed currently.
  // *
  // * @apiParam (JSON UL part parameters) {Number{0-255}} id The unique ID of
  // * the addressed UL slice.
  // * @apiParam (JSON UL part parameters) {String="xMBB","URLLC","mMTC","xMTC","Other"} [label]
  // * A descriptive label for this slice. It has currently no impact on the
  // * slice itself but might be extended to provide appropriate default values
  // * in the future.
  // * @apiParam (JSON UL part parameters) {Number{1-100}} [percentage] The
  // * number of resource blocks that this slice is allowed to use, as a fraction
  // * of the whole bandwidth. Please note that the sum of all slices is not
  // * allowed to exceed 100.
  // * @apiParam (JSON UL part parameters) {Boolean} [isolation] Whether this
  // * slice is isolated with regard to others. Refer to the
  // * `intersliceShareActive` to see the interaction with this parameter. If a
  // * slice is not isolated, unused RBs will be shared with other slices in the
  // * inter-slice multiplexing phase.
  // * @apiParam (JSON UL part parameters) {Number{0..20}} [priority] The
  // * priority of the slice when scheduling in the interslice multiplexing stage
  // * allocating in a greedy manner. Higher priority means preferential
  // * scheduling.
  // * @apiParam (JSON UL part parameters) {Number{0-$(bandwidth RB-1)}} [firstRb] Used to
  // * position a UL slice together with the percentage in the frequency plane.
  // * This parameter should be used to isolate slices in the UL and is subject
  // * to admission control like percentage: it is checked that no UL slice
  // * overlaps with any other, starting at `firstRb`
  // * and expanding `percentage` * bandwidth RB. This paramater is in *RB*,
  // * unlike the `positionLow` and `positionHigh` parameters in the UL.
  // * @apiParam (JSON UL part parameters) {Number{0-20}} [maxmcs] The maximum
  // * MCS that this slice is allowed to use.
  // * @apiParam (JSON UL part parameters) {String="POLU_FAIR","POLU_GREEDY"} [accounting]
  // * The algorithm used in the accounting phase, i.e. when allocating the
  // * resources to the UEs after having sorted them with respect to the
  // * `sorting` parameter.
  // * @apiParam (JSON UL part parameters) {String="schedule_ulsch_rnti"} [schedulerName]
  // * The name of the scheduler to be loaded. Can not be changed currently.
  // *
  // * @apiDescription This API endpoint posts a new slice configuration to an
  // * underlying agent, specified as a JSON file with the format of the
  // * `sliceConfig` as contained in the `cellConfig` of an agent configuration
  // * (for a description of the parameters, see below).  It can be used to
  // * create arbitrary slices with an arbitrary ID  or to change slices by
  // * specifying an ID for an existing slice. In the request, a slice ID must
  // * present. All other values will be copied from slice ID 0 if they are not
  // * present.  The caller should take care that the sum of slice percentages
  // * (i.e. of all present and added slices) should not exceed 100 and no UL
  // * slices overlap (a UL slice starts at `firstRb` and extends over
  // * `percentage` * bandwidth RBs. The `stats` call should always be used after
  // * a call and sufficient time to verify the actions have been taken.
  // *
  // * @apiVersion v0.1.0
  // * @apiPermission None
  // * @apiExample Example usage:
  // *    curl -X POST http://127.0.0.1:9999/slice/enb/-1 --data-binary "@file"
  // *
  // * @apiParamExample {json} Request-Example:
  // *    {
  // *      "dl": [
  // *        {
  // *          "id": 0,
  // *          "percentage": 25,
  // *          "maxmcs": 26
  // *        },
  // *        {
  // *          "id": 3,
  // *          "percentage": 25,
  // *          "maxmcs": 26
  // *        }
  // *      ],
  // *      "ul": [
  // *        {
  // *          "id": 0,
  // *          "percentage": 25,
  // *          "maxmcs": 16
  // *        },
  // *        {
  // *          "id": 3,
  // *          "percentage": 25,
  // *          "maxmcs": 18,
  // *          "firstRb": 25
  // *        }
  // *      ],
  // *      "intrasliceShareActive": true,
  // *      "intersliceShareActive": true
  // *    }
  // *
  // * @apiSuccessExample Success-Response:
  // *    HTTP/1.1 200 OK
  // *
  // * @apiError BadRequest Mal-formed request or missing/wrong parameters,
  // * reported as JSON.
  // *
  // * @apiErrorExample Error-Response:
  // *    HTTP/1.1 400 BadRequest
  // *    { "error": "can not find agent" }
  // *
  // * @apiErrorExample Error-Response:
  // *    HTTP/1.1 400 BadRequest
  // *    { "error": "missing slice ID" }
  // *
  // * @apiErrorExample Error-Response:
  // *    HTTP/1.1 400 BadRequest
  // *    { "error": "Protobuf parser error" }
  // */
  aqm_calls.route(desc.post("/enb/:id?"),
                  "Post a new AQM/SDAP configuration")
           .bind(&flexran::north_api::aqm_calls::apply_aqm_config, this);

  aqm_calls.route(desc.post("/enb/:enb/rnti/:ue?"),
                  "Post a new AQM/SDAP configuration")
           .bind(&flexran::north_api::aqm_calls::apply_aqm_config_rnti, this);
}

void flexran::north_api::aqm_calls::apply_aqm_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string bs = request.hasParam(":id") ?
      request.param(":id").as<std::string> () : "-1";

  const std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!aqm_app->handle_aqm_config(bs, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::aqm_calls::apply_aqm_config_rnti(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string bs = request.param(":enb").as<std::string>();
  const std::string ue = request.hasParam(":ue") ?
      request.param(":ue").as<std::string> () : "-1";

  const std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!aqm_app->handle_aqm_config_rnti(bs, ue, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

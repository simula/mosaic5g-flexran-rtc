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

/*! \file    rrm_calls.cc
 *  \brief   NB API for flexible_scheduler/RRM policies
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>

#include "flexran_log.h"
#include "rrm_calls.h"

void flexran::north_api::rrm_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto rrm_calls = desc.path("");

  /**
   * @api {post} /slice/enb/:id? _Deprecated Slice Configuration
   * @apiName DeprApplySliceConfiguration
   * @apiGroup SliceConfiguration
   * @apiDeprecated This description is deprecated and only for informational,
   * "historic" purposes. See <a
   * href="#api-SliceConfiguration-ApplySliceConfiguration">here instead</a>.
   *
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the agent for which
   * to change the slice configuration. This can be one of the following: -1
   * (last added agent), the eNB ID (in hex or decimal) or the internal agent
   * ID which can be obtained through a `stats` call. Numbers smaller than 1000
   * are parsed as the agent ID.
   *
   * @apiParam (JSON parameters) {Boolean} [intrasliceShareActive] The
   * activation status of the intra-slice sharing phase. If active, RBs that
   * are available after the first intra-slice allocation will be allocated to
   * UEs belonging to the same slice that need it.
   * @apiParam (JSON parameters) {Boolean} [intersliceShareActive] The
   * activation status of the inter-slice multiplexing phase. If active, RBs
   * that are available after the allocation of a slice, will be allocated to
   * UEs belonging to the other slices that need it. Isolated slices are always
   * ignored in this phase.
   * @apiParam (JSON parameters) {Object[]} [dl] A list of DL slice
   * configuration objects. For its parameters, see the parameters in the `JSON
   * DL part parameters` table below.
   * @apiParam (JSON parameters) {Object[]} [ul] A list of UL slice
   * configuration objects. For its parameters, see the parameters in the `JSON
   * UL part parameters` table below.
   *
   * @apiParam (JSON DL part parameters) {Number{0-255}} id The unique ID of
   * the addressed DL slice.
   * @apiParam (JSON DL part parameters) {String="xMBB","URLLC","mMTC","xMTC","Other"} [label]
   * A descriptive label for this slice. It has currently no
   * impact on the slice itself but might be extended to provide appropriate
   * default values in the future.
   * @apiParam (JSON DL part parameters) {Number{1-100}} [percentage] The
   * number of resource blocks that this slice is allowed to use, as a fraction
   * of the whole bandwidth. Please note that the sum of all slices is not
   * allowed to exceed 100.
   * @apiParam (JSON DL part parameters) {Boolean} [isolation] Whether this
   * slice is isolated with regard to others. Refer to the
   * `intersliceShareActive` to see the interaction with this parameter. If a
   * slice is not isolated, unused RBs will be shared with other slices in the
   * inter-slice multiplexing phase.
   * @apiParam (JSON DL part parameters) {Number{0-20}} [priority] The priority
   * of the slice when scheduling in the interslice multiplexing stage
   * allocating in a greedy manner. Higher priority means preferential
   * scheduling.
   * @apiParam (JSON DL part parameters) {Number{0-24}} [positionLow] When
   * positioning a slice in the frequency plane, this parameter marks the lower
   * end (inclusive, i.e. `posL <= RB`).  Expressed in *RBG*. Must be lower
   * than `positionHigh`. During the multiplexing phase, other RBs could be
   * chosen, too. See `intersliceShareActive` for more information.
   * @apiParam (JSON DL part parameters) {Number{1-25}} [positionHigh] When
   * positioning a slice in the frequency plane, this parameter marks the high
   * end (inclusive, i.e. `RB <= posH`).  Expressed in *RBG*. Must be higher
   * than `positionLow`. During the multiplexing phase, other RBs could be
   * chosen, too. See `intersliceShareActive` for more information.
   * @apiParam (JSON DL part parameters) {Number{0-28}} [maxmcs] The maximum
   * MCS that this slice is allowed to use.
   * @apiParam (JSON DL part parameters) {String[]="CR_ROUND","CR_SRB12","CR_HOL","CR_LC","CR_CQI","CR_LCP"} [sorting]
   * The policy by which users within a slice will be sorted before scheduling.
   * * `"CR_ROUND"`: Highest HARQ first.
   * * `"CR_SRB12"`: Highest SRB1+2 first.
   * * `"CR_HOL"`:   Highest HOL first.
   * * `"CR_LC"`:    Highest RLC buffer first.
   * * `"CR_CQI"`:   Highest CQI first.
   * * `"CR_LCP"`:   Highest LC priority first.
   * @apiParam (JSON DL part parameters) {String="POL_FAIR","POL_GREEDY"} [accounting]
   * The algorithm used in the accounting phase, i.e. when allocating the
   * resources to the UEs after having sorted them with respect to the
   * `sorting` parameter.
   * @apiParam (JSON DL part parameters) {String="schedule_ue_spec"} [schedulerName]
   * The name of the scheduler to be loaded. Can not be changed currently.
   *
   * @apiParam (JSON UL part parameters) {Number{0-255}} id The unique ID of
   * the addressed UL slice.
   * @apiParam (JSON UL part parameters) {String="xMBB","URLLC","mMTC","xMTC","Other"} [label]
   * A descriptive label for this slice. It has currently no impact on the
   * slice itself but might be extended to provide appropriate default values
   * in the future.
   * @apiParam (JSON UL part parameters) {Number{1-100}} [percentage] The
   * number of resource blocks that this slice is allowed to use, as a fraction
   * of the whole bandwidth. Please note that the sum of all slices is not
   * allowed to exceed 100.
   * @apiParam (JSON UL part parameters) {Boolean} [isolation] Whether this
   * slice is isolated with regard to others. Refer to the
   * `intersliceShareActive` to see the interaction with this parameter. If a
   * slice is not isolated, unused RBs will be shared with other slices in the
   * inter-slice multiplexing phase.
   * @apiParam (JSON UL part parameters) {Number{0..20}} [priority] The
   * priority of the slice when scheduling in the interslice multiplexing stage
   * allocating in a greedy manner. Higher priority means preferential
   * scheduling.
   * @apiParam (JSON UL part parameters) {Number{0-$(bandwidth RB-1)}} [firstRb] Used to
   * position a UL slice together with the percentage in the frequency plane.
   * This parameter should be used to isolate slices in the UL and is subject
   * to admission control like percentage: it is checked that no UL slice
   * overlaps with any other, starting at `firstRb`
   * and expanding `percentage` * bandwidth RB. This paramater is in *RB*,
   * unlike the `positionLow` and `positionHigh` parameters in the UL.
   * @apiParam (JSON UL part parameters) {Number{0-20}} [maxmcs] The maximum
   * MCS that this slice is allowed to use.
   * @apiParam (JSON UL part parameters) {String="POLU_FAIR","POLU_GREEDY"} [accounting]
   * The algorithm used in the accounting phase, i.e. when allocating the
   * resources to the UEs after having sorted them with respect to the
   * `sorting` parameter.
   * @apiParam (JSON UL part parameters) {String="schedule_ulsch_rnti"} [schedulerName]
   * The name of the scheduler to be loaded. Can not be changed currently.
   *
   * @apiDescription This description is out-dated and a new format has been
   * adopted. In the new endpoint, a number of the old parameters are not
   * available anymore, while new ones have been added. In particular, the new
   * system is capable of supporting arbitrary slice algorithms (they will be
   * added gradually over time), and the actual scheduling algorithm can be
   * set per slice.  On the other hand, the following parameters cannot be set
   * anymore or have changed:
   *
   * * `intersliceShareActive`: governed whether slices shared resources. Now
   * depends on the slice algorithm.
   * * `intrasliceShareActive`: governed whether user scheduling shared
   * resources. Now depends on the user scheduling algorithm.
   * * `percentage`: static slicing has a minimum granularity of a RBG (DL)/RB
   * (UL). As such, a percentage was misleading, and the lower/upper bounds of a
   * slice have to be set manually.
   * * `isolation`: defined whether this slices shares resources. Now depends
   * on the slice algorithm.
   * * `priority`: defined the priority of a slice during slice resource
   * sharing. Now depends on the slice algorithm.
   * * `maxmcs`: defined a maximum MCS. Now depends on the user scheduler.
   * Different schedulers might be defined that would apply a maximum MCS.
   * * `sorting`: sorting of users within a slice. Now depends on the user
   * scheduler.
   * * `accounting`: how resources are attributed to users. `POL_FAIR` is
   * closest to `round_robin_dl`, `POL_GREEDY` is closest to
   * `maximum_throughput_wbcqi_dl` together with `CR_LC` sorting.
   * * `firstRb`: a starting position of a slice in UL. Has been superseded
   * by `posLow`. a call and sufficient time to verify the actions have been taken.
   *
   * Original description: This API endpoint posts a new slice configuration to an
   * underlying agent, specified as a JSON file with the format of the
   * `sliceConfig` as contained in the `cellConfig` of an agent configuration
   * (for a description of the parameters, see below).  It can be used to
   * create arbitrary slices with an arbitrary ID  or to change slices by
   * specifying an ID for an existing slice. In the request, a slice ID must
   * present. All other values will be copied from slice ID 0 if they are not
   * present.  The caller should take care that the sum of slice percentages
   * (i.e. of all present and added slices) should not exceed 100 and no UL
   * slices overlap (a UL slice starts at `firstRb` and extends over
   * `percentage` * bandwidth RBs. The `stats` call should always be used
   * after triggering this endpoint and sufficient time to verify the actions
   * have been taken.
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
   *          "maxmcs": 26
   *        },
   *        {
   *          "id": 3,
   *          "percentage": 25,
   *          "maxmcs": 26
   *        }
   *      ],
   *      "ul": [
   *        {
   *          "id": 0,
   *          "percentage": 25,
   *          "maxmcs": 16
   *        },
   *        {
   *          "id": 3,
   *          "percentage": 25,
   *          "maxmcs": 18,
   *          "firstRb": 25
   *        }
   *      ],
   *      "intrasliceShareActive": true,
   *      "intersliceShareActive": true
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
  /**
   * @api {post} /slice/enb/:id? Post a slice configuration
   * @apiName ApplySliceConfiguration
   * @apiGroup SliceConfiguration
   *
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the agent for which
   * to change the slice configuration. This can be one of the following: -1
   * (last added agent), the eNB ID (in hex or decimal) or the internal agent
   * ID which can be obtained through a `stats` call. Numbers smaller than 1000
   * are parsed as the agent ID.
   *
   * @apiParam (JSON parameters) {Object} [dl] The slicing/scheduler
   * configuration in DL.
   * @apiParam (JSON parameters) {Object} [ul] The slicing/scheduler
   * configuration in UL.
   *
   * @apiParam (DL Parameters) {String=None,Static} [algorithm] The DL
   * slicing algorithm (where `None` means no slicing).
   * @apiParam (DL Parameters) {Object[]} [slices] A list of slices to set or
   * modify. Not compatible with `None` (no slicing algorithm).
   * @apiParam (DL Parameters) {Number{1-255}} slices[id] Mandatory ID of this DL
   * slice.
   * @apiParam (DL Parameters) {String} [slices[label]] An optional label.
   * @apiParam (DL Parameters) {String="round_robin_dl","proportional_fair_wbcqi_dl","maximum_throughput_wbcqi_dl"} [slices[scheduler]] The scheduler to use for this slice.
   * @apiParam (DL Parameters) {Object} [slices[static]] The parameters for the
   * `Static` slicing algorithm. See "DL Static Slicing".
   * @apiParam (DL Static Slicing) {Number} [static[posLow]] The lower
   * (inclusive) starting resource block group (RBG) for this slice. It should
   * not overlap with any other existing or new slice. See "DL Static Slicing"
   * @apiParam (DL Static Slicing) {Number} [static[posHigh]] The upper
   * (inclusive!) starting resource block group (RBG) for this slice. It should
   * not overlap with any other existing or new slice.
   * @apiParam (DL Parameters) {String="round_robin_dl","proportional_fair_wbcqi_dl","maximum_throughput_wbcqi_dl"} [slices[scheduler]] The scheduler to use in case of no slicing algorithm. Only compatible with `None` (no slicing algorithm).
   *
   * @apiParam (UL Parameters) {String=None,Static} [algorithm] The UL
   * slicing algorithm (where `None` means no slicing algorithm).
   * @apiParam (UL Parameters) {Object[]} [slices] A list of slices to set or
   * modify. Not compatible with `None` (no slicing algorithm).
   * @apiParam (UL Parameters) {Number{1-255}} slices[id] Mandatory ID of this UL
   * slice.
   * @apiParam (UL Parameters) {String} [slices[label]] An optional label.
   * @apiParam (UL Parameters) {String="round_robin_ul"} [slices[scheduler]] The scheduler to use for this slice.
   * @apiParam (UL Parameters) {Object} [slices[static]] The parameters for the
   * `Static` slicing algorithm. See "UL Static Slicing".
   * @apiParam (UL Static Slicing) {Number} [static[posLow]] The lower
   * (inclusive) starting resource block (RB) for this slice. It should not
   * overlap with any other existing or new slice.
   * @apiParam (UL Static Slicing) {Number} [static[posHigh]] The upper
   * (inclusive!) starting resource block (RB) for this slice. It should
   * not overlap with any other existing or new slice.
   * @apiParam (UL Parameters) {String="round_robin_ul"} [slices[scheduler]] The scheduler to use in case of no slicing algorithm. Only compatible with `None` (no slicing algorithm).
   *
   * @apiDescription This API endpoint posts a new slice configuration to an
   * underlying agent, specified as a JSON file with the format of the
   * `sliceConfig` as contained in the `cellConfig` of an agent configuration
   * (for a description of the parameters, see below), or the scheduler to use
   * if no slicing algorithm is chosen (`None`).  It can be used to
   * create arbitrary slices with an arbitrary ID or to change slices by
   * specifying an ID for an existing slice. In the request, a slice ID must be
   * present, as well as the slice parameters. The label is optional, and if no
   * scheduler is given, the scheduler that was active when enabling slicing
   * will be used. The `stats` call should always be used after triggering this
   * endpoint and sufficient time to verify the actions have been taken. Note
   * that the `scheduler` (within `dl`/`ul`) and the `slices` parameters are
   * mutually exclusive, since the first only applies for no slicing algorithm
   * (`None`), whereas the latter only applies if a slicing algorithm has been
   * chosen.
   *
   * This API call has changed as of July 2020. For a description of how the
   * old parameters can be reproduced in the new call, see <a
   * href="#api-SliceConfiguration-DeprApplySliceConfiguration">Deprecated
   * Slice Configuration</a>
   *
   * Remarks on the `Static` slicing algorithm: this algorithm does not provide
   * any sharing. Note that `posLow`/`posHigh` are in terms of resource block
   * groups (RBG) in DL and resource blocks (RB) in UL. Furthermore, it is not
   * checked that the channel bandwidth can actually accommodate this slice
   * (`posLow=100` and `posHigh=110` are a valid entry, but they will never
   * work, since LTE has a maximum bandwidth of 100RBs). Please refer to the
   * `stats` call to check the amount of resource blocks as well as the
   * resource block group size which will tell the applicable RB/RBG settings.
   * Please also note that OAI reserves the first and last 1, 2, or 3 RBs (for
   * bandwidths 25, 50, or 100RBs, respectively) for PUCCH, meaning that these
   * first/last RBs should be spared out/they won't be given to the slice.
   * Also, the minimum RB size in UL is 3!
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/slice/enb/-1 --data-binary "@file"
   *
   * @apiParamExample {json} Static Slicing in DL:
   *     {
   *       "dl": {
   *         "algorithm": "Static",
   *         "slices": [
   *           {
   *             "id": 0,
   *             "static": {
   *               "posLow": 4,
   *               "posHigh": 12
   *             }
   *           },
   *           {
   *             "id": 2,
   *             "static": {
   *               "posLow": 0,
   *               "posHigh": 3
   *             }
   *           }
   *         ]
   *       },
   *       "ul": {
   *         "algorithm": "None"
   *       }
   *     }
   *
   * @apiParamExample {json} PF scheduler in DL, no slicing:
   *     {
   *       "dl": {
   *         "scheduler": "proportional_fair_wbcqi_dl"
   *       }
   *     }
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
  rrm_calls.route(desc.post("/slice/enb/:id?"),
                  "Post a new slice configuration")
           .bind(&flexran::north_api::rrm_calls::apply_slice_config, this);

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
   * for a given agent.  A valid slice ID must present. Slice 0 can not be
   * removed. To remove the slice algorithm, consider posting the `None` slice
   * algorithm instead.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X DELETE http://127.0.0.1:9999/slice/enb/-1 --data-binary "@file"
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "dl": {
   *        "slices": [
   *          {
   *            "id": 3,
   *          }
   *        ]
   *      }
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
  rrm_calls.route(desc.del("/slice/enb/:id?"),
                  "Delete slices as specified in the JSON")
           .bind(&flexran::north_api::rrm_calls::remove_slice_config, this);

  /**
   * @api {post} /ue_slice_assoc/enb/:id? Change the UE-slice association
   * @apiName ChangeUeSliceAssociation
   * @apiGroup SliceConfiguration
   *
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the agent for which
   * to change the slice configuration. This can be one of the following: -1
   * (last added agent), the eNB ID (in hex or decimal) or the internal agent
   * ID which can be obtained through a `stats` call. Numbers smaller than 1000
   * are parsed as the agent ID.
   *
   * @apiParam (JSON parameter) {Object[]} ueConfig A list of UE-slice
   * association configuration parameters (see table `ueConfig parameters`
   * below.
   *
   * @apiParam (ueConfig parameters) {Number} [rnti] The RNTI for the selected
   * UE. If both `rnti` and `imsi` are given, they need to match the same UE.
   * @apiParam (ueConfig parameters) {Number} [imsi] The IMSI for the selected
   * UE. If both `rnti` and `imsi` are given, they need to match the same UE.
   * @apiParam (ueConfig parameters) {Number{0-255}} [dlSliceId] The Dl slice
   * to which this UE should be associated to.
   * @apiParam (ueConfig parameters) {Number{0-255}} [ulSliceId] The Ul slice
   * to which this UE should be associated to.
   *
   * @apiDescription This API endpoint changes the association of a UE in an
   * underlying agent, specified as a JSON file with the format of the
   * `ueConfig` as contained in the agent configuration.  It can be used to
   * changed the association of UEs using their current RNTI or IMSI. In the
   * request, a slice ID and RNTI or IMSI must be present. The `stats` call
   * should always be used after triggering this endpoint and sufficient time
   * to verify the actions have been taken.
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
  rrm_calls.route(desc.post("/ue_slice_assoc/enb/:id?"),
                  "Change the slice association of a UE")
           .bind(&flexran::north_api::rrm_calls::change_ue_slice_assoc, this);

  /**
   * @api {post} /auto_ue_slice_assoc/enb/:id?/slice/:slice_id/:dir? Automate the UE-slice association
   * @apiName AutoAssociateUESlice
   * @apiGroup SliceConfiguration
   *
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the agent for which
   * to change the slice configuration. This can be one of the following: -1
   * (last added agent), the eNB ID (in hex or decimal) or the internal agent
   * ID which can be obtained through a `stats` call. Numbers smaller than 1000
   * are parsed as the agent ID.
   * @apiParam (URL parameter) {Number} slice_id The slice that UEs shall be
   * associated to.
   * @apiParam (URL parameter) {String="both","dl","ul"} [dir="both"] The slice
   * that UEs shall be associated to.
   *
   * @apiParam (JSON parameter) {String[]} json A simple JSON list of regexes
   * matching IMSIs as string. For backwards-compatibility, also recognizes
   * IMSIs as numbers.
   *
   * @apiDescription This API saves a list of IMSIs or regular expressions
   * matching on IMSIs in order to auto-associate those to a particular slice
   * when they connect. It is checked that the base station has a slice with
   * the given slice ID. Whenever a UE whose IMSI is known (this might not
   * always be the case, go to flight mode and exit to get it reliably) is not
   * in the slice it was associated to, it will automatically be associated to
   * this slice. When associating a new list to a slice which already has an
   * association list, that list will be removed.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/auto_ue_slice_assoc/enb/-1/slice/3/dl --data-binary "@imsi_list.json"
   *
   * @apiParamExample {json} Request-Example:
   *    [ "^20895" ]
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "no slices found" }
   */
  rrm_calls.route(desc.post("/auto_ue_slice_assoc/enb/:id?/slice/:slice_id/:dir?"),
                  "Associate a list of IMSIs to a particular slice")
           .bind(&flexran::north_api::rrm_calls::auto_ue_slice_assoc, this);

  /**
   * @api {post} /conf/enb/:id? Change the cell configuration (restart)
   * @apiName CellReconfiguration
   * @apiGroup CellConfigurationPolicy
   *
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the agent for which
   * to change the slice configuration. This can be one of the following: -1
   * (last added agent), the eNB ID (in hex or decimal) or the internal agent
   * ID which can be obtained through a `stats` call. Numbers smaller than 1000
   * are parsed as the agent ID.
   *
   * @apiParam (JSON parameters) {Number=6,15,25,50,100} dlBandwidth The DL
   * bandwidth on which the eNB should operate. This should be the same as the
   * UL bandwidth.
   * @apiParam (JSON parameters) {Number=6,15,25,50,100} ulBandwidth The UL
   * bandwidth on which the eNB should operate. This should be the same as the
   * UL bandwidth.
   * @apiParam (JSON parameters) {Number} dlFreq The DL frequency on which the
   * eNB should operate, subject to the chosen bandwith (see below). Check also
   * the [LTE frequency bands Wikipedia
   * article](https://en.wikipedia.org/wiki/LTE_frequency_bands) for allowed
   * values regarding range and frequency offsets.
   * @apiParam (JSON parameters) {Number} ulFreq The UL frequency on which the
   * eNB should operate, subject to the chosen bandwith (see below). Check also
   * the [LTE frequency bands Wikipedia
   * article](https://en.wikipedia.org/wiki/LTE_frequency_bands) for allowed
   * values regarding range and frequency offsets.
   * @apiParam (JSON parameters) {Number=7,13} eutraBand The LTE band in which
   * the base station will operate.
   *
   * @apiDescription This API endpoint changes the cell configuration of the
   * eNodeB in the underlying agent, effectively resulting in soft-restart of
   * the base station (only L1/L2/L3 will be restarted). The parameters are
   * specified as a JSON file with the format of the `cellConfig` as contained
   * in the agent configuration, but only the parametrs `dlBandwidth`,
   * `ulBandwidth`, `dlFreq`, `ulFreq` end `eutraBand` are accepted.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/conf/enb/-1 --data-binary "@file"
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "dlBandwidth": 50,
   *      "ulBandwidth": 50,
   *      "dlFreq": 2650,
   *      "ulFreq": 2530,
   *      "eutraBand": 7
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
   *    { "error": "freq offset must be 120MHz for band 7" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "unrecognized band 1" }
   */
  rrm_calls.route(desc.post("/conf/enb/:id?"),
                  "Change the cell configuration of the eNodeB")
           .bind(&flexran::north_api::rrm_calls::cell_reconfiguration, this);

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
  rrm_calls.route(desc.post("/yaml/:id?"),
                  "Deprecated: Send arbitrary YAML file")
           .bind(&flexran::north_api::rrm_calls::yaml_compat, this);
}

void flexran::north_api::rrm_calls::apply_slice_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs = "-1";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  try {
    rrm_app->apply_slice_config_policy(bs, policy);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void flexran::north_api::rrm_calls::remove_slice_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs = "-1";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  try {
    rrm_app->remove_slice(bs, policy);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void flexran::north_api::rrm_calls::change_ue_slice_assoc(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs = "-1";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  try {
    rrm_app->change_ue_slice_association(bs, policy);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void flexran::north_api::rrm_calls::yaml_compat(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  uint64_t bs_id = request.hasParam(":id") ?
      rrm_app->parse_enb_agent_id(request.param(":id").as<std::string>()) :
      rrm_app->get_last_bs();
  if (bs_id == 0) {
    response.send(Pistache::Http::Code::Not_Found, "Policy not set (no such BS)\n");
    return;
  }
  if (request.body().length() == 0) {
    response.send(Pistache::Http::Code::Not_Found, "Policy not set (body is empty)\n");
    return;
  }

  LOG4CXX_INFO(flog::app, "sending YAML request to BS " << bs_id
      << " (compat):\n" << request.body());
  rrm_app->reconfigure_agent_string(bs_id, request.body());
  response.send(Pistache::Http::Code::Ok,
                "Set the policy to BS " + std::to_string(bs_id) + "\n");
}

void flexran::north_api::rrm_calls::auto_ue_slice_assoc(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs = "-1";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  const uint32_t slice_id = request.param(":slice_id").as<uint32_t>();
  std::string dir = "both";
  if (request.hasParam(":dir")) dir = request.param(":dir").as<std::string>();
  int32_t dl_slice_id = -1;
  int32_t ul_slice_id = -1;
  if (dir == "both") {
    dl_slice_id = slice_id;
    ul_slice_id = slice_id;
  } else if (dir == "dl") {
    dl_slice_id = slice_id;
  } else if (dir == "ul") {
    ul_slice_id = slice_id;
  } else {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"unrecognized direction '" + dir + "'\" }",
        MIME(Application, Json));
  }

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  try {
    rrm_app->auto_ue_slice_association(bs, policy, dl_slice_id, ul_slice_id);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void flexran::north_api::rrm_calls::cell_reconfiguration(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  uint64_t bs_id = request.hasParam(":id") ?
      rrm_app->parse_enb_agent_id(request.param(":id").as<std::string>()) :
      rrm_app->get_last_bs();
  if (bs_id == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find BS\" }", MIME(Application, Json));
    return;
  }

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!rrm_app->apply_cell_config_policy(bs_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

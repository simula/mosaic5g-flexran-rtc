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
   * @apiDescription This API endpoint posts a new slice configuration to an
   * underlying agent, specified as a JSON file with the format of the
   * `sliceConfig` as contained in the `cellConfig` of an agent configuration
   * (for a description of the parameters, see below).  It can be used to
   * create arbitrary slices with an arbitrary ID  or to change slices by
   * specifying an ID for an existing slice. In the request, a slice ID must
   * present. All other values will be copied from slice ID 0 if they are not
   * present.  The caller should take care that the sum of slice percentages
   * (i.e. of all present and added slices) should not exceed 100 and no UL
   * slices overlap (a UL slice starts at `firstRb` and extends over
   * `percentage` * bandwidth RBs. The `stats` call should always be used after
   * a call and sufficient time to verify the actions have been taken.
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
  rrm_calls.route(desc.post("/slice/enb/:id?"),
                  "Post a new slice configuration")
           .bind(&flexran::north_api::rrm_calls::apply_slice_config, this);

  /**
   * @api {post} /slice/enb/:id? Create a new pair of slices (short version)
   * @apiName ApplySliceConfigurationShort
   * @apiGroup SliceConfiguration
   * @apiParam {Number} id The ID of the agent for which to change the slice
   * configuration. This can be one of the following: -1 (last added agent),
   * the eNB ID (in hex or decimal) or the internal agent ID which can be
   g obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam {Number{1-255}} slice_id The ID of the slices (UL *and* DL) to
   * be created.
   *
   * @apiDescription This API endpoint creates a new pair of slices copying the
   * values of the slice 0.  It can be used to create arbitrary slices with an
   * arbitrary ID. Please note that if slice 0 has already more than 50
   * percent, this call will fail (since the percentage value is copied, too).
   * The caller should take care that the sum of slice percentages (i.e. of all
   * present and added slices) should not exceed 100.  Additionally, the
   * `firstRb` parameter is added so that the added UL slice will pass the
   * admission control (not overlapping). This value is retrieved by taking the
   * `percentage` of slice 0 and the highest slice's `firstRb` values and
   * setting `firstRb` + `percentage` * bandwidth as this slice's `firstRb`.
   * The `stats` call should always be used after a call and sufficient time to
   * verify the actions have been taken. If it fails, use the long version of
   * this calls.
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
  rrm_calls.route(desc.post("/slice/enb/:id/slice/:slice_id"),
                  "Create a new pair of slices copying the values of slice 0")
           .bind(&flexran::north_api::rrm_calls::apply_slice_config_short, this);

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
  rrm_calls.route(desc.del("/slice/enb/:id?"),
                  "Delete slices as specified in the JSON")
           .bind(&flexran::north_api::rrm_calls::remove_slice_config, this);

  /**
   * @api {delete} /slice/enb/:id/slice/:slice_id Delete slices (short version)
   * @apiName DeleteSliceShort
   * @apiGroup SliceConfiguration
   * @apiParam {Number} id The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam {Number{1-255}} slice_id The ID of the slices (UL *and* DL) to be
   * created.
   *
   * @apiDescription This API endpoint deletes the UL *and* DL slices with ID
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
  rrm_calls.route(desc.del("/slice/enb/:id/slice/:slice_id"),
                  "Delete the UL and DL slices by ID")
           .bind(&flexran::north_api::rrm_calls::remove_slice_config_short, this);

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
  rrm_calls.route(desc.post("/ue_slice_assoc/enb/:id?"),
                  "Change the slice association of a UE")
           .bind(&flexran::north_api::rrm_calls::change_ue_slice_assoc, this);

  /**
   * @api {post} /ue_slice_assoc/enb/:enb_id/ue/:rnti_imsi/slice/:slice_id Change the UE-slice association (short version)
   * @apiName ChangeUeSliceAssociationShort
   * @apiGroup SliceConfiguration
   * @apiParam {Number} enb_id The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam {Number} rnti_imsi The ID of the UE in the form of either an
   * RNTI or the IMSI. Everything shorter than 6 digits will be treated as the
   * RNTI, the rest as the IMSI.
   * @apiParam {Number} slice_id The ID of the slices in UL *and* DL to which the
   * UE should be changed.
   *
   * @apiDescription This API endpoint changes the association of a UE to a
   * *pair* of slices (UL *and* DL) in an underlying agent. In particular, the
   * destination slices for UL and DL must have the same ID (there is no short
   * hand to change only a UL or DL slice association). It can be used to
   * changed the association of UEs using their current RNTI or IMSI. The
   * UL and DL slices with the given ID must have been created before. The
   * `stats` call should always be used after a call and sufficient time to
   * verify that the actions have been taken.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/ue_slice_assoc/enb/-1/ue/208940100001115/slice/3
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
   *    { "error": "can not find UE for given agent" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "no such slice" }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Protobuf parser error" }
   */
  rrm_calls.route(desc.post("/ue_slice_assoc/enb/:enb_id/ue/:rnti_imsi/slice/:slice_id"),
                  "Change the association of a UE to a UL-DL slice pair (same ID)")
           .bind(&flexran::north_api::rrm_calls::change_ue_slice_assoc_short, this);

  /**
   * @api {post} /install_vnetwork/:bps Create a virtual network
   * @apiName InstallVNetwork
   * @apiGroup SliceConfiguration
   *
   * @apiParam {Number} bps The desired bit rate every slice should fulfill
   *
   * @apiDescription This API endpoint installs a virtual network, i.e. a slice
   * on every connected base station with a common slice ID and with a
   * percentage of resources such that every slice can deliver up the given
   * bitrate under the assumption of MCS 28 in DL and MCS 20 in UL. The
   * end-point call will calculate a percentage, rounded up to a full RBG, and
   * send the appropriate command to every connected base station. On success,
   * the slice ID that is common to all BS will be returned. Please note that
   * individual agents might refuse the creation of a slice and the presence of
   * all slices has to be checked independently.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/install_vnetwork/2000000
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    { "slice_id": 8 }
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "BS 12345 cannot provide the requested bitrate" }
   */
  rrm_calls.route(desc.post("/install_vnetwork/:bps"),
                  "Install a virtual network, i.e. a new slice on every connected base station")
           .bind(&flexran::north_api::rrm_calls::instantiate_vnetwork, this);

  /**
   * @api {post} /remove_vnetwork/:slice_id Remove a virtual network
   * @apiName RemoveVNetwork
   * @apiGroup SliceConfiguration
   *
   * @apiParam {Number} slice_id The slice ID of the vnetwork
   *
   * @apiDescription This API endpoint remove a virtual network, i.e. a slice
   * on every connected base station with a common slice ID. The end-point
   * checks that every base station has such a slice and removes it from all
   * of them. Note that the free space won't be filled by any other slice and
   * has to be reconfigured manually. The controller will remove the UE-slice
   * association list entries for this slice.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/remove_vnetwork/13
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Mal-formed request or missing/wrong parameters,
   * reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "BS 12345 does not have slice 13" }
   */
  rrm_calls.route(desc.post("/remove_vnetwork/:slice_id"),
                  "Remove a virtual network")
           .bind(&flexran::north_api::rrm_calls::remove_vnetwork, this);

  /**
   * @api {post} /associate_ue_vnetwork/:slice_id Associate users to a slice
   * @apiName AssociateUEVNetwork
   * @apiGroup SliceConfiguration
   *
   * @apiParam (URL parameter) {Number} slice_id The slice/vnetwork that UEs
   * shall be associated to.
   *
   * @apiParam (JSON parameter) {Number[]} json A simple JSON list of IMSIs as
   * numbers.
   *
   * @apiDescription This API associates a list of IMSIs to a particular
   * vnetwork/slice. Internally, it is checked that at least one base station
   * has a slice with the given slice ID and can therefore be used without a
   * prior call to <a
   * href="#api-SliceConfiguration-InstallVNetwork">SliceConfiguration:InstallVNetwork</a>. In this
   * case, the IMSIs are stored.  Whenever a UE whose IMSI is known (this might
   * not always be the case, go to flight mode and exit to get it reliably) is
   * not in the slice it was associated to, it will automatically be associated
   * to this slice. The controller checks for this every second.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/associate_ue_vnetwork/1 --data-binary "@imsi_list.json"
   *
   * @apiParamExample {json} Request-Example:
   *    [ 208950000000007, 208950000000107 ]
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
  rrm_calls.route(desc.post("/associate_ue_vnetwork/:slice_id"),
                  "Associate a list of IMSIs to a particular slice")
           .bind(&flexran::north_api::rrm_calls::associate_ue_vnetwork, this);

  /**
   * @api {post} /remove_ue_vnetwork/ Remove user-slice association by IMSI
   * @apiName RemoveUEListVNetwork
   * @apiGroup SliceConfiguration
   *
   * @apiParam (JSON parameter) {Number[]} json A simple JSON list of IMSIs as
   * numbers.
   *
   * @apiDescription This API removes all UE-slice associations for a given
   * list of IMSIs. It returns the number of removed UEs.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/remove_ue_vnetwork/ --data-binary "@imsi_list.json"
   *
   * @apiParamExample {json} Request-Example:
   *    [ 208950000000007, 208950000000107 ]
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    { "removed_items": 2 }
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "parser error" }
   */
  rrm_calls.route(desc.post("/remove_ue_vnetwork/"),
                  "Remove all UE-slice associations for a list of IMSIs")
           .bind(&flexran::north_api::rrm_calls::remove_ue_list_vnetwork, this);

  /**
   * @api {post} /remove_ue_vnetwork/:slice_id Remove user-slice association by slice
   * @apiName RemoveUEVNetwork
   * @apiGroup SliceConfiguration
   *
   * @apiParam (URL parameter) {Number} slice_id The slice/vnetwork whose
   * UE-slice association shall be deleted.
   *
   * @apiDescription This API removes all UE IMSIs that are associated to a
   * given slice ID. It returns the number of removed UEs. The request body is
   * ignored.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://127.0.0.1:9999/remove_ue_vnetwork/1
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    { "removed_items": 1 }
   */
  rrm_calls.route(desc.post("/remove_ue_vnetwork/:slice_id"),
                  "Remove all UE-slice associations for a slice")
           .bind(&flexran::north_api::rrm_calls::remove_ue_vnetwork, this);


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
  if (!rrm_app->apply_slice_config_policy(bs_id, policy, error_reason)) {
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
  uint64_t bs_id = rrm_app->parse_enb_agent_id(request.param(":id").as<std::string>());
  if (bs_id == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find BS\" }", MIME(Application, Json));
    return;
  }

  int slice_id = request.param(":slice_id").as<int>();
  std::string policy;
  policy  = "{\"dl\":[{id:" + std::to_string(slice_id);
  policy += "}],\"ul\":[{id:" + std::to_string(slice_id);
  policy += "}]}";

  std::string error_reason;
  if (!rrm_app->apply_slice_config_policy(bs_id, policy, error_reason)) {
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
  if (!rrm_app->remove_slice(bs_id, policy, error_reason)) {
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
  uint64_t bs_id = rrm_app->parse_enb_agent_id(request.param(":id").as<std::string>());
  if (bs_id == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find BS\" }", MIME(Application, Json));
    return;
  }

  int slice_id = request.param(":slice_id").as<int>();
  std::string policy;
  policy  = "{\"dl\":[{id:" + std::to_string(slice_id);
  policy += ",percentage:0}],\"ul\":[{id:" + std::to_string(slice_id);
  policy += ",percentage:0}]}";

  std::string error_reason;
  if (!rrm_app->remove_slice(bs_id, policy, error_reason)) {
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
  if (!rrm_app->change_ue_slice_association(bs_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::rrm_calls::change_ue_slice_assoc_short(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  uint64_t bs_id = rrm_app->parse_enb_agent_id(request.param(":enb_id").as<std::string>());
  if (bs_id == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find BS\" }", MIME(Application, Json));
    return;
  }

  /* the RNTI/IMSI will be validated but we can not be sure what of the two it
   * is, so let the RIB figure it out. We will receive a valid RNTI to proceed,
   * or an error */
  flexran::rib::rnti_t rnti;
  if (!rrm_app->parse_rnti_imsi(bs_id, request.param(":rnti_imsi").as<std::string>(), rnti)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find UE for given BS\" }", MIME(Application, Json));
    return;
  }

  /* the slice ID will be validated later, but make sure it is a number */
  int slice_id = request.param(":slice_id").as<int>();

  std::string policy = "{\"ueConfig\":[{\"rnti\":";
  policy += std::to_string(rnti) + ",\"dlSliceId\":";
  policy += std::to_string(slice_id) + ",\"ulSliceId\":";
  policy += std::to_string(slice_id) + "}]}";

  std::string error_reason;
  if (!rrm_app->change_ue_slice_association(bs_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::rrm_calls::instantiate_vnetwork(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const uint64_t bps = request.param(":bps").as<uint64_t>();
  std::string error_reason;
  const int slice_id = rrm_app->instantiate_vnetwork(bps, error_reason);
  if (slice_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok,
      "{ \"slice_id\": " + std::to_string(slice_id) + " }", MIME(Application, Json));
}

void flexran::north_api::rrm_calls::remove_vnetwork(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const uint32_t slice_id = request.param(":slice_id").as<uint32_t>();
  std::string error_reason;
  if (!rrm_app->remove_vnetwork(slice_id, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::rrm_calls::associate_ue_vnetwork(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const uint32_t slice_id = request.param(":slice_id").as<uint32_t>();

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!rrm_app->associate_ue_vnetwork(slice_id, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::rrm_calls::remove_ue_list_vnetwork(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  const int removed = rrm_app->remove_ue_vnetwork(policy, error_reason);
  if (removed < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok,
      "{ \"removed_items\": " + std::to_string(removed) + " }");
}

void flexran::north_api::rrm_calls::remove_ue_vnetwork(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const uint32_t slice_id = request.param(":slice_id").as<uint32_t>();
  const int removed = rrm_app->remove_ue_vnetwork(slice_id);
  response.send(Pistache::Http::Code::Ok,
      "{ \"removed_items\": " + std::to_string(removed) + " }");
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

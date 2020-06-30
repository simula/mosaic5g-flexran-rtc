/*
 * Copyright 2016-2020 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    plmn_calls.cc
 *  \brief   NB API for PLMN and MME control
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>

#include "flexran_log.h"
#include "plmn_calls.h"

void flexran::north_api::plmn_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto mme_calls = desc.path("/mme");

  /**
   * @api {post} /mme/enb/:id? Add a new MME
   * @apiName AddMme
   * @apiGroup PlmnManagement
   * @apiParam (URL parameter ) {Number} id The ID of the agent for which to
   * add the MME. This can be one of the following: -1 (last added agent), the
   * eNB ID (in hex or decimal) or the internal agent ID which can be obtained
   * through a `stats` call. Numbers smaller than 1000 are parsed as the agent
   * ID.
   * @apiParam (JSON parameter) {Object[]} mme A list of (objects of) MME IP
   * addresses to connect to. Only one MME can be configured at a time.
   * @apiParam (mme parameters) {String} s1Ip The IP address of the MME to
   * connect to.
   *
   * @apiDescription This API endpoint triggers the base station to connect to
   * a new MME (and therefore, to a new core network).  The list of PLMNs
   * should be configured such that the PLMN(s) served by this new CN are
   * present in the RAN (either through eNB configuration, or dynamically
   * through the <a href="#api-PlmnManagement-PushPLMNs">corresponding
   * north-bound call</a>). Note that a registration with a subset of the PLMNs
   * (e.g., to hide a PLMN from the CN) is not possible.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -XPOST localhost:9999/mme/enb/ --data-binary @mme.json
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "mme": [
   *        {
   *          "s1Ip": "192.168.12.4"
   *        }
   *      ]
   *    }
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Missing or wrong parameters, reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "MME at IP 192.168.12.4 already present" }
   */
  mme_calls.route(desc.post("/enb/:id?"),
                  "Post an MME configuration")
           .bind(&flexran::north_api::plmn_calls::add_mme, this);

  /**
   * @api {delete} /mme/enb/:id? Remove an MME
   * @apiName RemoveMme
   * @apiGroup PlmnManagement
   * @apiParam (URL parameter ) {Number} id The ID of the agent for which to
   * remove the MME. This can be one of the following: -1 (last added agent),
   * the eNB ID (in hex or decimal) or the internal agent ID which can be
   * obtained through a `stats` call. Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam (JSON parameter) {Object[]} mme A list of (objects of) MME IP
   * addresses to be removed. Only one MME can be configured at a time.
   * @apiParam (mme parameters) {String} s1Ip The IP address of the MME to
   * be removed.
   *
   * @apiDescription This API endpoint triggers the base station to disconnect
   * from one MME (and therefore, from one core network).  Note that it is
   * advisable to not disconnect when a UE is still connected. The controller
   * does not check whether UEs are still connected and will forward the
   * message regardless!
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -XDELETE localhost:9999/mme/enb/ --data-binary @mme.json
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "mme": [
   *        {
   *          "s1Ip": "192.168.12.4"
   *        }
   *      ]
   *    }
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *
   * @apiError BadRequest Missing or wrong parameters, reported as JSON.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "No MME at IP 192.168.12.4 present" }
   */
  mme_calls.route(desc.del("/enb/:id?"),
                  "Remove an MME")
           .bind(&flexran::north_api::plmn_calls::remove_mme, this);

  auto plmn_calls = desc.path("/plmn");

  /**
   * @api {post} /plmn/enb/:id? Push PLMNs
   * @apiName PushPLMNs
   * @apiGroup PlmnManagement
   * @apiParam (URL parameter ) {Number} id The ID of the agent for which to
   * push the new PLMNs. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed
   * as the agent ID.
   * @apiParam (JSON parameter) {Object[]} plmnId A list of PLMN IDs (as also
   * shown in the stats call).
   * @apiParam (plmnId parameters) {Number{100-999}} mcc The new mobile country code.
   * @apiParam (plmnId parameters) {Number{10-999}} mnc The new mobile network code.
   * @apiParam (plmnId parameters) {Number{2-3}} mncLength The length of the
   * `mnc` (needs to match).
   *
   * @apiDescription This API endpoint pushes a new set of PLMNs that are
   * broadcasted by the base station. The old PLMNs will be overwritten. Please
   * note that it is advisable to disconnect from MMEs that will not be served
   * anymore.
   *
   * @apiDescription This API endpoint pushes a new set of PLMNs that are
   * broadcasted by the base station. The old PLMNs will be overwritten. Please
   * note that it is advisable to disconnect from MMEs that will not be served
   * anymore.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -XPOST localhost:9999/plmn/enb --data-binary @plmn.json
   *
   * @apiParamExample {json} Request-Example:
   *    {
   *      "plmnId": [
   *        {
   *          "mcc": 208,
   *          "mnc": 95,
   *          "mncLength": 2
   *        },
   *        {
   *          "mcc": 208,
   *          "mnc": 94,
   *          "mncLength": 2
   *        }
   *      ]
   *    }
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
  plmn_calls.route(desc.post("/enb/:id?"),
                   "Post a new set of PLMNs")
            .bind(&flexran::north_api::plmn_calls::change_plmn, this);
}

void flexran::north_api::plmn_calls::add_mme(
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
    plmn_app->add_mme(bs, policy);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void flexran::north_api::plmn_calls::remove_mme(
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
    plmn_app->remove_mme(bs, policy);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void flexran::north_api::plmn_calls::change_plmn(
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
    plmn_app->change_plmn(bs, policy);
  } catch (const std::invalid_argument& e) {
    LOG4CXX_ERROR(flog::app, "encountered error while processing " << __func__
          << "(): " << e.what());
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

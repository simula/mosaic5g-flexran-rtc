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

/*! \file    netstore_loader_calls.cc
 *  \brief   NB Test API for NetStore trigger
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>

#include "rt_controller_common.h"
#include "flexran_log.h"
#include "netstore_loader_calls.h"

void flexran::north_api::netstore_loader_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto netstore = desc.path("/netstore");

  /**
   * @api {get} /netstore/app/:id Deploy Agent App
   * @apiName DeployAgentApp
   * @apiGroup NetStore
   *
   * @apiParam {string} id The name of the app to load.
   *
   * @apiDescription Deploys a FlexRAN agent app by contacting the NetStore and
   * searching an application of name `:id` within it. If found, it will deploy
   * this application on the base station/within the agent and start it. If not
   * found, nothing happens; check the FlexRAN log for this. Typically, the
   * agent writes the received object into the directory denoted by
   * `FLEXRAN_CACHE` in the OAI configuration file. The application expects the
   * NetStore to be reachable at `localhost:8080`.
   *
   * FlexRAN agent apps are applications that can be directly embedded within
   * the process. This is a security risk, as such application can do whatever
   * it wants, e.g., sniff traffic.
   *
   * Note that in order to use this end-point, you have to upload such
   * application to the NetStore. Run the NetStore with `python file_store.py`
   * within the Mosaic5G store. Compile the applications against the base
   * station: `make flapp_all` within the OAI build directory should build all
   * applications (this might happen while OAI is running!). Upload to the
   * NetStore like this:
   * ```
   * curl -XPOST localhost:8081/push/sample --data-binary @libflapp_sample.so
   * ```
   * It should then be possible to trigger the download.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl localhost:9999/netstore/app/sample
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    requested 'sample'
   */
  netstore.route(desc.get("/app/:id"), "Trigger app download")
      .bind(&flexran::north_api::netstore_loader_calls::app_dl_trigger, this);

  /**
   * @api {del} /netstore/app/:id Release Agent App
   * @apiName ReleaseAgentApp
   * @apiGroup NetStore
   *
   * @apiParam {string} id The name of the app to release.
   *
   * @apiDescription Release a previously loaded FlexRAN Agent app on the base
   * station. It will be stopped, and then removed from the `FLEXRAN_CACHE`.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -XDELETE localhost:9999/netstore/app/sample
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    requested removal 'sample'
   */
  netstore.route(desc.del("/app/:id"), "Remove app from agent")
          .bind(&flexran::north_api::netstore_loader_calls::app_rm_trigger, this);

  /**
   * @api {get} /netstore/app/:id Reconfigure App
   * @apiName ReconfigureApp
   * @apiGroup NetStore
   *
   * @apiParam {string} id The name of the app to reconfigure.
   *
   * @apiDescription Sends a reconfiguration request to a FlexRAN agent app.
   * Note that applications are reconfigured through a key-value dictionary.
   * Thus, the syntax of the reconfiguration file (see below) is fixed, but the
   * semantics is completely free!
   *
   * At the time of this writing, two applications exist that have the
   * following parameterization:
   * * sample: will print whatever key-value pair it receives.
   * * imsi: takes key-vals, e.g.,
   *    1. "regex" : "^20895"
   *    2. "slice_dl" : "3"
   *    3. "slice_ul" : "3"
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl localhost:9999/netstore/app/imsi -XPOST -d @imsi-reconfiguration.json
   * @apiExample IMSI reconfiguration JSON
   *     {
   *       "params" : {
   *         "regex": { "str": "^20894" },
   *         "slice_dl": { "integer": 3 }
   *         "slice_ul": { "integer": 3 }
   *       }
   *     }
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    requested reconfiguration of app 'sample'
   */
  netstore.route(desc.post("/app/:id"), "Reconfigure app at agent")
          .bind(&flexran::north_api::netstore_loader_calls::app_conf_trigger, this);
}

void flexran::north_api::netstore_loader_calls::app_dl_trigger(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string id = request.param(":id").as<std::string>();
  netstore->trigger_app_request(id);
  response.send(Pistache::Http::Code::Ok, "requested '" + id + "'\n");
}

void flexran::north_api::netstore_loader_calls::app_rm_trigger(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string id = request.param(":id").as<std::string>();
  netstore->trigger_app_stop(id);
  response.send(Pistache::Http::Code::Ok, "requested removal of app '" + id + "'\n");
}

void flexran::north_api::netstore_loader_calls::app_conf_trigger(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string id = request.param(":id").as<std::string>();
  std::string policy = request.body();
  try {
    netstore->trigger_app_reconfig(id, policy);
  } catch (const std::invalid_argument& e) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "requested reconfiguration of app '" + id + "'\n");
}

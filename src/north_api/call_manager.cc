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

/*! \file    call_manager.cc
 *  \brief   manager for HTTP server
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include "call_manager.h"
#include "rt_controller_common.h"
#include <pistache/router.h>
#include <iostream>

flexran::north_api::manager::call_manager::call_manager(Pistache::Address addr)
	: httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr)),
    desc_("FlexRAN NB API", RTC_VERSION)
{
  desc_.info()
       .license("Apache", "http://www.apache.org/licenses/LICENSE-2.0");

  desc_.schemes(Pistache::Rest::Scheme::Http)
       .basePath("/")
       .produces(MIME(Application, Json))
       .consumes(MIME(Application, Json));

  /**
   * @api {get} /capabilities Get API list
   * @apiName Capabilities
   * @apiGroup Capabilities
   *
   * @apiDescription This call lists the current active REST end-points in JSON
   * format, i.e.  the capabilities of the controller. It informs whether the
   * controller is running and the northbound interface is up. Furthermore, it
   * can be used to determine the active applications at the northbound
   * interface for specific purposes. The returned JSON consists of one info
   * object with generic information and a paths object representing a list of
   * endpoints, the HTTP method to retrieve information, and a short
   * description.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/capabilities | jq .
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    {
   *      "info": {
   *        "title": "FlexRAN NB API",
   *        "version": "2.0"
   *      },
   *      "paths": {
   *        "/capabilities": {
   *          "GET": {
   *            "description": "list active REST endpoints"
   *          }
   *        }
   *      }
   *    }
   */
  desc_.route(desc_.get("/capabilities"), "list active REST endpoints")
       .bind(&flexran::north_api::manager::call_manager::list_api, this);
}

void flexran::north_api::manager::call_manager::init(size_t thr) {
  auto opts = Pistache::Http::Endpoint::options().threads(thr)
      .flags(Pistache::Tcp::Options::ReuseAddr);
  httpEndpoint->init(opts);
}

void flexran::north_api::manager::call_manager::start()
{
  Pistache::Rest::Router router;
  router.initFromDescription(desc_);
  httpEndpoint->setHandler(router.handler());
  httpEndpoint->serveThreaded();
}

void flexran::north_api::manager::call_manager::shutdown() {
  httpEndpoint->shutdown();
}

void flexran::north_api::manager::call_manager::register_calls(flexran::north_api::app_calls& calls)
{
  calls.register_calls(desc_);
}

void flexran::north_api::manager::call_manager::list_api(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  _unused(request);
  /* creates list of API endpoints like so:
   * {
   *   "info": {
   *     "title": "Text",
   *     "version": "Text"
   *   },
   *   "paths": {
   *     "/endpointA": {
   *       "GET": {
   *         "description": "Text"
   *       },
   *       "POST": {
   *         ...
   *       }
   *     },
   *     ...
   *   }
   * }
   * Could be extended for Swagger documentation as described in
   * pistache/examples/rest_description.cc with real JSON serializer
   */
  std::string json = "{\"info\":{\"title\":\"";
  json += desc_.rawInfo().title;
  json += "\",\"version\":\"";
  json += desc_.rawInfo().version;
  json += "\"},\"paths\":{";
  auto paths = desc_.rawPaths();
  for (auto it = paths.flatBegin(), end = paths.flatEnd(); it != end; ++it) {
    const auto& ppaths = *it;
    if (it != paths.flatBegin()) json += ",";
    json += "\"" + ppaths.begin()->value + "\":{";
    for (auto p = ppaths.begin(), pend = ppaths.end(); p != pend; ++p) {
      if (p != ppaths.begin()) json += ",";
      json += "\"";
      json += Pistache::Http::methodString(p->method);
      json += "\":";
      json += "{\"description\":\"" + p->description + "\"}";
    }
    json += "}";
  }
  json += "}}\n";
  response.send(Pistache::Http::Code::Ok, json, MIME(Application, Json));
}

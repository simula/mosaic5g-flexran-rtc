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
    desc_("FlexRAN NB API", "2.0")
{
  desc_.info()
       .license("Apache", "http://www.apache.org/licenses/LICENSE-2.0");

  desc_.schemes(Pistache::Rest::Scheme::Http)
       .basePath("/")
       .produces(MIME(Application, Json))
       .consumes(MIME(Application, Json));

  desc_.route(desc_.get("/simple-flexran-api.json"),
              "list FlexRAN REST endpoints in a simple format")
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

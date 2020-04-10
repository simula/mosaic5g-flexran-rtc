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

  mme_calls.route(desc.post("/enb/:id?"),
                  "Post an MME configuration")
           .bind(&flexran::north_api::plmn_calls::add_mme, this);

  mme_calls.route(desc.del("/enb/:id?"),
                  "Remove an MME")
           .bind(&flexran::north_api::plmn_calls::remove_mme, this);
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

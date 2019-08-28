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

/*! \file    delegation_calls.cc
 *  \brief   NB API for delegation manager
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>
#include <fstream>
#include <vector>

#include "flexran_log.h"
#include "delegation_calls.h"

void flexran::north_api::delegation_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto dg = desc.path("/delegation");

  /**
   * @api {post} /body/enb/:id/name/:name Push an object (from REST)
   * @apiName PushObjectRest
   * @apiGroup Delegation
   *
   * @apiParam (URL parameter) {Number} The ID of the base station to which the
   * object should be sent. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed
   * as the agent ID.
   * @apiParam (URL parameter) {String} The name of the object to send. This
   * name can be freely chosen and is used to refer to the object later on.
   *
   * @apiParam (Request Body) {Binary} The (binary) shared object to be sent to
   * the agent. Its size has to be between 1 and 230000B.
   *
   * @apiDescription This API endpoint can be used to send a shared object to
   * the agent (e.g., SDAP/AQM scheduler). If the object is large, prefer the
   * file-based endpoint.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -XPOST localhost:9999/delegation/body/enb/-1/name/object --data-binary @/tmp/file
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
   */
  dg.route(desc.post("/body/enb/:id/name/:name"),
                  "Push a shared object from HTTP body to a controller")
    .bind(&flexran::north_api::delegation_calls::push_object, this);

  /**
   * @api {post} /file/enb/:id/name/:name Push an object (from file)
   * @apiName PushObjectFile
   * @apiGroup Delegation
   *
   * @apiParam (URL parameter) {Number} The ID of the base station to which the
   * object should be sent. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex or decimal) or the internal agent ID which can
   * be obtained through a `stats` call. Numbers smaller than 1000 are parsed
   * as the agent ID.
   * @apiParam (URL parameter) {String} The name of the object to send. This
   * name can be freely chosen and is used to refer to the object later on.
   *
   * @apiParam (Request Body) {String} The filename of the object to send. Its
   * size has to be between 1 and 230000B.
   *
   * @apiDescription This API endpoint can be used to send a shared object to
   * the agent (e.g., SDAP/AQM scheduler) based on a file name.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -XPOST localhost:9999/delegation/file/enb/-1/name/object --data "/tmp/file/
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
   */
  dg.route(desc.post("/file/enb/:id/name/:name"),
                  "Push a shared object from a file to an agent")
    .bind(&flexran::north_api::delegation_calls::load_object, this);
}

void flexran::north_api::delegation_calls::push_object(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string bs = request.param(":id").as<std::string>();
  const std::string name = request.param(":name").as<std::string>();

  const std::string policy = request.body();
  const int len = policy.length();
  if (len == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!delegation_app->push_object(bs, name, policy.c_str(), len, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

void flexran::north_api::delegation_calls::load_object(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  const std::string bs = request.param(":id").as<std::string>();
  const std::string name = request.param(":name").as<std::string>();

  const std::string filename = request.body();
  if (filename.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  std::ifstream file{filename, std::ios::binary};
  if (!file.is_open()) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"cannot open file " + filename + "\" }\n", MIME(Application, Json));
    return;
  }
  std::vector<unsigned char> buf(std::istreambuf_iterator<char>(file), {});
  const char *data = reinterpret_cast<char*>(buf.data());

  std::string error_reason;
  if (!delegation_app->push_object(bs, name, data, buf.size(), error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "");
}

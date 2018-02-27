/* The MIT License (MIT)

   Copyright (c) 2018 Robert Schmidt

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <pistache/http.h>
#include <pistache/http_header.h>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>

#include "recorder_calls.h"

void flexran::north_api::recorder_calls::register_calls(Pistache::Rest::Router& router)
{
  /*
   * usage: POST http://address:port/record/[:type/[:duration]] (type and
   * duration are optional)
   * possible types: all, enb, stats, bin
   * duration: in milliseconds
   */
  Pistache::Rest::Routes::Post(router, "/record/:type?/:duration?",
      Pistache::Rest::Routes::bind(&flexran::north_api::recorder_calls::start_meas, this));

  /*
   * usage: GET http://address:port/record/:ID, GET (ID is mandatory)
   */
  Pistache::Rest::Routes::Get(router, "/record/:id",
      Pistache::Rest::Routes::bind(&flexran::north_api::recorder_calls::obtain_json_stats,
        this));
}

void flexran::north_api::recorder_calls::start_meas(const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string type = request.hasParam(":type") ? request.param(":type").as<std::string>() : "all";
  uint32_t duration = request.hasParam(":duration") ? request.param(":duration").as<uint32_t>() : 1000;

  std::string id;
  bool success = json_app->start_meas(duration, type, id);
  if (success) {
    response.send(Pistache::Http::Code::Ok, id);
  } else {
    /* 409 Bad Conflict -> Server state conflict -> can not handle now */
    response.send(Pistache::Http::Code::Conflict, "Can not handle request at the moment\n");
  }
}

void flexran::north_api::recorder_calls::obtain_json_stats(const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string id = request.param(":id").as<std::string>();
  // dummy initialization
  flexran::app::log::job_info info{0, 0, id, flexran::app::log::job_type::all};
  if (!json_app->get_job_info(id, info)) {
    response.send(Pistache::Http::Code::Bad_Request, "Invalid ID (no such job)\n");
    return;
  }

  std::ifstream file;

  if (info.type == flexran::app::log::job_type::bin)
    file.open(info.filename, std::ios::binary);
  else
    file.open(info.filename);

  if (!file.is_open()) {
    response.send(Pistache::Http::Code::Bad_Request,
        "Corresponding file " + info.filename + " could not be opened\n");
    return;
  }

  std::stringstream ss;
  ss << file.rdbuf();
  file.close();
  response.send(Pistache::Http::Code::Ok, ss.str());
}

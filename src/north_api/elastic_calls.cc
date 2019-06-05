/*
 * Copyright 2016-2019 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    elastic_calls.h
 *  \brief   NB API for elastic search app
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>

#include "rt_controller_common.h"
#include "flexran_log.h"
#include "elastic_calls.h"

#include <chrono>
#include <iomanip>

void flexran::north_api::elastic_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto elastic = desc.path("/elasticmon");

  /**
   * @api {get} /elasticmon Status
   * @apiName Status
   * @apiGroup ElasticMonitoring
   *
   * @apiDescription Returns the the current status/configuration for the
   * ElasticMon application in JSON.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl http://localhost:9999/elasticmon | jq .
   *
   * @apiSuccess {Boolean} active Indicates whether logging is enabled (true)
   * or disabled (false).
   * @apiSuccess {String} activeSince Indicates since when the current logging
   * session is active. Not shown if logging is disabled.
   * @apiSuccess {Number} sentPackets The number of request packets that have
   * been successfully sent to ElasticSearch in the currently or last active
   * logging session, i.e. for which HTTP response code 200 has been received.
   * Note that multiple samples might be sent in one batch, equalling one
   * packet.
   * @apiSuccess {[String]} endpoint The list of endpoints to which information
   * will be streamed in the form `IP:Port`.
   * @apiSuccess {Number} intervalStats The interval between two consecutive
   * statistics samples (in ms), i.e. samples for dynamic UE-related
   * information. See also `batchStatsMaxSize`.
   * @apiSuccess {Number} intervalConfig The interval between two consecutive
   * configuration samples (in ms), i.e. samples for static eNB- or UE-related
   * information. see also `batchConfigMaxSize`.
   * @apiSuccess {Number} batchStatsMaxSize The number of statistics samples
   * that are collected before being sent to the ElasticSearch instance.
   * Put differently, after `intervalStats * batchStatsMaxSize` ms, a request
   * containing all statistics samples is sent to the ElasticSearch instance.
   * @apiSuccess {Number} batchConfigMaxSize The number of configuration
   * samples that are collected before being sent to the ElasticSearch
   * instance. Put differently, after `intervalConfig * batchConfigMaxSize` ms,
   * a request containing all configuration samples is sent to the
   * ElasticSearch instance.
   *
   * @apiSuccessExample Success-Response:
   *    HTTP/1.1 200 OK
   *    {
   *      "active": true,
   *      "activeSince": "2019-02-18 14:55:11",
   *      "sentPackets": 22,
   *      "endpoint": [
   *        "localhost:9200"
   *      ],
   *      "intervalStats": 100,
   *      "intervalConfig": 5000
   *      "batchStatsMaxSize": 100,
   *      "batchConfigMaxSize": 5
   *    }
   */
  elastic.route(desc.get("/"),
                "Return the current status/configuration")
         .bind(&flexran::north_api::elastic_calls::status, this);

  /**
   * @api {post} /elasticmon/endpoint/:ep Add ElasticSearch endpoint
   * @apiName AddEndpoint
   * @apiGroup ElasticMonitoring
   * @apiParam {string} ep The endpoint in the form `IP:Port`
   *
   * @apiDescription Posts a new endpoint of a running ElasticSearch instance
   * to the ElasticMon application. The endpoint needs to be in the form
   * `IP:Port` where IP is either a numerical IPv4 address or a hostname. More
   * specifically, it needs to match the regex
   * `^(?:[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+|[a-zA-Z0-9-]+):\d+$`. Note that it is
   * not not actually checked that the endpoint exists (e.g. via sending a
   * request).
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/endpoint/192.168.12.45:11213
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The given endpoint is not of the correct form
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "illegal endpoint or already present"}
   */
  elastic.route(desc.post("/endpoint/:ep"),
                "Post a new endpoint of a running ElasticSearch instance")
         .bind(&flexran::north_api::elastic_calls::add_endpoint, this);

  /**
   * @api {delete} /elasticmon/endpoint/:ep Remove ElasticSearch endpoint
   * @apiName RemoveEndpoint
   * @apiGroup ElasticMonitoring
   * @apiParam {string} ep The endpoint in the form `IP:Port`
   *
   * @apiDescription Removes an existing endpoint of a running ElasticSearch instance
   * in the ElasticMon application. If the removed endpoint was the last one
   * and logging was active, it is disabled.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X DELETE http://localhost:9999/elasticmon/endpoint/192.168.12.45:11213
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The given endpoint does not exist
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "is the endpoint present?"}
   */
  elastic.route(desc.del("/endpoint/:ep"),
                "Remove an existing endpoint of a running ElasticSearch instance")
         .bind(&flexran::north_api::elastic_calls::remove_endpoint, this);

  /**
   * @api {post} /elasticmon/interval/stats/:itvl Set statistics monitoring interval
   * @apiName IntervalStats
   * @apiGroup ElasticMonitoring
   * @apiParam {number{0-1000}} itvl The sampling interval for statistics in ms.
   *
   * @apiDescription Sets the sampling interval (in ms) for sending statistics
   * to the ElasticSearch server. Statistics refers to the dynamic information
   * collected from various layers and related to a certain UE. A parameter
   * value of 0 switches statistics sampling off. If logging is active, the new
   * interval will be set by *restarting* the corresponding timer. For
   * instance, if the old interval was 50ms and the new interval of 30ms is
   * set at 40ms (i.e., 10ms before the old sampling interval), the next
   * sample will be at millisecond 70 from the initial last sample.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/interval/stats/50
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The sampling interval is illegal
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Is the interval in the range 0<=dt<=1000?"}
   */
  elastic.route(desc.post("/interval/stats/:itvl"),
                "Set the sampling interval (in ms) for sending statistics")
         .bind(&flexran::north_api::elastic_calls::set_freq_stats, this);

  /**
   * @api {post} /elasticmon/interval/config/:itvl Set configuration monitoring interval
   * @apiName IntervalConfig
   * @apiGroup ElasticMonitoring
   * @apiParam {number{0-}} itvl The sampling interval for configuration in ms.
   *
   * @apiDescription Sets the sampling interval (in ms) for sending configuration
   * to the ElasticSearch server. Configuration refers to the static state of
   * the RAN entities (eNB, UE, LCs). A value of 0 switches configuration
   * sampling off. If logging is active, the new interval will be set by
   * *restarting* the corresponding timer. For instance, if the old interval
   * was 5000ms (=5s) and the new interval of 3000ms (=3s) is set at 4000ms
   * (i.e., 1 s before the old sampling interval), the next sample will be at
   * second 7.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/interval/config/10000
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The sampling interval is illegal
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Is the interval in the range 0<=dt?"}
   */
  elastic.route(desc.post("/interval/config/:itvl"),
                "Set the sampling interval (in ms) for sending configuration")
         .bind(&flexran::north_api::elastic_calls::set_freq_config, this);

  /**
   * @api {post} /elasticmon/batch/stats/:size Set statistics batch size
   * @apiName BatchStatsSize
   * @apiGroup ElasticMonitoring
   * @apiParam {number{1-1000}} size The number of statistics samples to be
   * sent to ElasticSearch at once.
   *
   * @apiDescription Sets the batch size for sending statistics, i.e. the
   * number of samples that are to be sent in one request to the ElasticSearch
   * instance. This parameter can be used to tune the number of requests to be
   * sent to the ElasticSearch instance per second and their size in case of
   * real-time problems.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/batch/stats/10
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The batch size is illegal
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Is the batch size in the range 1<=size<=1000?"}
   */
  elastic.route(desc.post("/batch/stats/:size"),
                "Set the batch size for sending statistics")
         .bind(&flexran::north_api::elastic_calls::set_batch_stats_size, this);

  /**
   * @api {post} /elasticmon/batch/config/:size Set configuration batch size
   * @apiName BatchConfigSize
   * @apiGroup ElasticMonitoring
   * @apiParam {number{1-1000}} size The number of configuration samples to be
   * sent to ElasticSearch at once.
   *
   * @apiDescription Sets the batch size for sending configuration, i.e. the
   * number of samples that are to be sent in one request to the ElasticSearch
   * instance. This parameter can be used to tune the number of requests to be
   * sent to the ElasticSearch instance per second and their size in case of
   * real-time problems.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/batch/config/10
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The batch size is illegal
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Is the batch size in the range 1<=size<=1000?"}
   */
  elastic.route(desc.post("/batch/config/:size"),
                "Set the batch size for sending configuration")
         .bind(&flexran::north_api::elastic_calls::set_batch_config_size, this);

  /**
   * @api {post} /elasticmon/enable Enable ElasticMon logging
   * @apiName Enable
   * @apiGroup ElasticMonitoring
   *
   * @apiDescription Enables the ElasticMon logging framework. Note that this
   * might notably fail because the endpoint is not configured properly or
   * because logging is already active.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/enable
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest The endpoint might be wrong or logging is already
   * enabled
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "could not enable logging"}
   */
  elastic.route(desc.post("/enable"),
                "Enable the ElasticMon logging framework")
         .bind(&flexran::north_api::elastic_calls::enable, this);

  /**
   * @api {post} /elasticmon/disable Disable ElasticMon logging
   * @apiName Disable
   * @apiGroup ElasticMonitoring
   *
   * @apiDescription Disables the ElasticMon logging framework. Note that this
   * might fail if logging is already disabled.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X POST http://localhost:9999/elasticmon/disable
   *
   * @apiSuccessExample Success-Response
   *     HTTP/1.1 200 OK
   *
   * @apiError BadRequest An unknown error occurred or already disabled
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    { "error": "could not disable logging"}
   */
  elastic.route(desc.post("/disable"),
                "Disable the ElasticMon logging framework")
         .bind(&flexran::north_api::elastic_calls::disable, this);
}

void flexran::north_api::elastic_calls::status(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  _unused(request);
  /* provide a JSON representation of the most important parameters */
  std::string s;
  std::vector<std::string> eps = elastic_app->get_endpoint();
  s  = "{";
  s +=   "\"active\":"; /* need to end statement here, otherwise it is cut */
  s +=                                elastic_app->is_active() ? "true" : "false";
  if (elastic_app->is_active()) {
    const auto t = std::chrono::system_clock::to_time_t(elastic_app->get_active_since());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%F %T");
    s +=  ",\"activeSince\":\""     + ss.str() + "\"";
  }
  s +=  ",\"sentPackets\":"         + std::to_string(elastic_app->get_sent_packets());
  s +=   ",\"endpoint\":[";
  for (auto it = eps.begin(); it != eps.end(); it++) {
    if (it != eps.begin()) s += ",";
    s +=    "\"" + *it + "\"";
  }
  s +=   "],\"intervalStats\":"    + std::to_string(elastic_app->get_freq_stats());
  s +=   ",\"intervalConfig\":"     + std::to_string(elastic_app->get_freq_config());
  s +=   ",\"batchStatsMaxSize\":"  + std::to_string(elastic_app->get_batch_stats_max_size());
  s +=   ",\"batchConfigMaxSize\":" + std::to_string(elastic_app->get_batch_config_max_size());
  s += "}";
  response.send(Pistache::Http::Code::Ok, s);
}

void flexran::north_api::elastic_calls::add_endpoint(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string ep = request.param(":ep").as<std::string>();
  if (elastic_app->add_endpoint(ep))
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"illegal endpoint or already present\"}", MIME(Application, Json));
}

void flexran::north_api::elastic_calls::remove_endpoint(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string ep = request.param(":ep").as<std::string>();
  if (elastic_app->remove_endpoint(ep))
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"is the endpoint present?\"}", MIME(Application, Json));
}

void flexran::north_api::elastic_calls::set_freq_stats(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int freq = request.param(":itvl").as<int>();
  if (elastic_app->set_freq_stats(freq))
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"Is the interval in the range 0<=dt<=1000?\"}",
        MIME(Application, Json));
}

void flexran::north_api::elastic_calls::set_freq_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int freq = request.param(":itvl").as<int>();
  if (elastic_app->set_freq_config(freq))
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"Is the interval in the range 0<=dt?\"}",
        MIME(Application, Json));
}
void flexran::north_api::elastic_calls::set_batch_stats_size(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int size = request.param(":size").as<int>();
  if (elastic_app->set_batch_stats_max_size(size))
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"Is the batch size in the range 1<=size<=1000?\"}",
        MIME(Application, Json));
}

void flexran::north_api::elastic_calls::set_batch_config_size(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  int size = request.param(":size").as<int>();
  if (elastic_app->set_batch_config_max_size(size))
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"Is the batch size in the range 1<=size<=1000?\"}",
        MIME(Application, Json));
}


void flexran::north_api::elastic_calls::enable(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  _unused(request);
  if (elastic_app->enable_logging())
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"could not enable logging\"}",
        MIME(Application, Json));
}

void flexran::north_api::elastic_calls::disable(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  _unused(request);
  if (elastic_app->disable_logging())
    response.send(Pistache::Http::Code::Ok, "");
  else
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"could not disable logging\"}",
        MIME(Application, Json));
}

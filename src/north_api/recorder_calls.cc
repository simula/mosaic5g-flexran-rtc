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

  /**
     * @api {post} record/[:type/[:duration]]  Record realtime RAN statistic
     * @apiName record
     * @apiGroup Stats
     * @apiParam {string} type Available types are : all(default), enb, stats, bin
     * @apiParam {string} duration Duration of record in millisecond, must be a positive value. Default value is 1000
     * @apiDescription This API request FlexRAN RTC to record the RAN stats of given type for a given duration. Both type and duration parameters are optional.
     * @apiVersion v0.1.0
     * @apiPermission None
     * @apiExample Example usage:
     *     curl -X POST http://127.0.0.1:9999/record
     *     curl -X POST http://127.0.0.1:9999/record/stats
     *     curl -X POST http://127.0.0.1:9999/record/enb/1
     * @apiSuccessExample Success-Response:
     *     HTTP/1.1 200 OK
     *     {
     *       "id" : val
     * }
     * @apiError TypeNotFound The stats type not found.
     *
     */


  /*
   * usage: GET http://address:port/record/:ID, GET (ID is mandatory)
   */
  Pistache::Rest::Routes::Get(router, "/record/:id",
      Pistache::Rest::Routes::bind(&flexran::north_api::recorder_calls::obtain_json_stats,
        this));

    /**
     * @api {get} record/:id  Get the recorded realtime RAN statistic by id
     * @apiName record
     * @apiGroup Stats
     * @apiParam {int} id identifier of the previously recorded RAN stats
     * @apiDescription This API gets the  FlexRAN RTC to record the RAN stats of given type for a given duration. Both type and duration parameters are optional.
     * @apiVersion v0.1.0
     * @apiPermission None
     * @apiExample Example usage:
     *     curl -X GET http://127.0.0.1:9999/record/1
     * @apiSuccessExample Success-Response:
     *     HTTP/1.1 200 OK
     *
     * "eNB_config":[
     *   {
     *       "eNB":{
     *           "header":{
     *               "version":0,
     *               "type":8,
     *               "xid":0
     *           },
     *           "cellConfig":[
     *               {
     *                   "cellId":0,
     *                   "puschHoppingOffset":0,
     *                  "hoppingMode":0,
     *                   "nSb":1,
     *                   "phichResource":0,
     *                   "phichDuration":0,
     *                   "dlBandwidth":25,
     *                   "ulBandwidth":25,
     *                   "ulCyclicPrefixLength":0,
     *                   "dlCyclicPrefixLength":0,
     *                   "antennaPortsCount":1,
     *                   "duplexMode":1,
     *                   "prachConfigIndex":0,
     *                   "prachFreqOffset":2,
     *                   "raResponseWindowSize":7,
     *                   "macContentionResolutionTimer":5,
     *                   "maxHARQMsg3Tx":0,
     *                   "n1PUCCHAN":0,
     *                   "deltaPUCCHShift":1,
     *                   "nRBCqi":0,
     *                   "srsSubframeConfig":0,
     *                   "srsBwConfig":0,
     *                   "srsMacUpPts":0,
     *                   "enable64QAM":0,
     *                   "carrierIndex":0,
     *                   "dlFreq":2685,
     *                   "ulFreq":2565,
     *                   "eutraBand":7,
     *                   "dlPdschPower":-27,
     *                   "ulPuschPower":-96
     *               }
     *           ]
     *       },
     *       "UE":{
     *           "header":{
     *               "version":0,
     *               "type":10,
     *               "xid":1
     *           },
     *           "ueConfig":[
     *               {
     *                   "rnti":9902,
     *                   "timeAlignmentTimer":7,
     *                   "transmissionMode":0,
     *                   "ueAggregatedMaxBitrateUL":"0",
     *                   "ueAggregatedMaxBitrateDL":"0",
     *                   "capabilities":{
     *                   },
     *                   "ueTransmissionAntenna":2,
     *                   "ttiBundling":0,
     *                   "maxHARQTx":4,
     *                   "betaOffsetACKIndex":0,
     *                   "betaOffsetRIIndex":0,
     *                   "betaOffsetCQIIndex":8,
     *                   "ackNackSimultaneousTrans":0,
     *                   "simultaneousAckNackCqi":0,
     *                   "aperiodicCqiRepMode":3,
     *                   "ackNackRepetitionFactor":0,
     *                   "pcellCarrierIndex":0
     *               }
     *           ]
     *       },
     *       "LC":{
     *           "header":{
     *               "version":0,
     *               "type":12,
     *               "xid":2
     *           },
     *           "lcUeConfig":[
     *               {
     *                   "rnti":9902,
     *                   "lcConfig":[
     *                       {
     *                           "lcid":1,
     *                           "direction":2,
     *                           "qosBearerType":0,
     *                           "qci":1
     *                       },
     *                       {
     *                           "lcid":2,
     *                           "direction":2,
     *                           "qosBearerType":0,
     *                           "qci":1
     *                       },
     *                       {
     *                           "lcid":3,
     *                           "direction":1,
     *                           "qosBearerType":0,
     *                           "qci":1
     *                       }
     *                   ]
     *               }
     *           ]
     *       }
     *   }
     *],
     * "mac_stats":[
     *   {
     *       "agent_id":0,
     *       "ue_mac_stats":[
     *           {
     *               "rnti":9902,
     *               "mac_stats":{
     *                   "bsr":[
     *                       0,
     *                       20,
     *                       0,
     *                       0
     *                   ],
     *                   "phr":32,
     *                   "rlcReport":[
     *                       {
     *                           "lcId":1,
     *                           "txQueueSize":0,
     *                           "txQueueHolDelay":0,
     *                           "statusPduSize":0
     *                       },
     *                       {
     *                           "lcId":2,
     *                           "txQueueSize":0,
     *                           "txQueueHolDelay":0,
     *                           "statusPduSize":0
     *                       },
     *                       {
     *                           "lcId":3,
     *                           "txQueueSize":0,
     *                           "txQueueHolDelay":0,
     *                           "statusPduSize":0
     *                       }
     *                   ],
     *                   "pendingMacCes":0,
     *                   "dlCqiReport":{
     *                       "sfnSn":9556,
     *                       "csiReport":[
     *                           {
     *                               "servCellIndex":0,
     *                               "ri":0,
     *                               "type":"FLCSIT_P10",
     *                               "p10csi":{
     *                                   "wbCqi":15
     *                               }
     *                           }
     *                       ]
     *                   },
     *                   "ulCqiReport":{
     *                       "sfnSn":9556,
     *                       "cqiMeas":[
     *                           {
     *                               "type":"FLUCT_SRS",
     *                               "servCellIndex":0
     *                           }
     *                       ],
     *                       "pucchDbm":[
     *                           {
     *                               "p0PucchDbm":0,
     *                               "servCellIndex":0
     *                           }
     *                       ]
     *                   },
     *                   "rrcMeasurements":{
     *                       "measid":-1,
     *                       "pcellRsrp":-92,
     *                       "pcellRsrq":3
     *                   },
     *                   "pdcpStats":{
     *                       "pktTx":119,
     *                       "pktTxBytes":44799,
     *                       "pktTxSn":118,
     *                       "pktTxW":0,
     *                       "pktTxBytesW":0,
     *                       "pktTxAiat":21693,
     *                       "pktTxAiatW":0,
     *                       "pktRx":145,
     *                       "pktRxBytes":38610,
     *                       "pktRxSn":144,
     *                       "pktRxW":0,
     *                       "pktRxBytesW":0,
     *                       "pktRxAiat":21726,
     *                       "pktRxAiatW":0,
     *                       "pktRxOo":0,
     *                       "sfn":"26451"
     *                   },
     *                   "macStats":{
     *                       "tbsDl":1287,
     *                       "tbsUl":63,
     *                       "prbRetxDl":0,
     *                       "prbRetxUl":0,
     *                       "prbDl":14,
     *                       "prbUl":0,
     *                       "mcs1Dl":28,
     *                       "mcs2Dl":28,
     *                       "mcs1Ul":10,
     *                       "mcs2Ul":10,
     *                       "totalBytesSdusUl":118088,
     *                       "totalBytesSdusDl":45559,
     *                       "totalPrbDl":621,
     *                       "totalPrbUl":5421,
     *                       "totalPduDl":105,
     *                       "totalPduUl":1655,
     *                       "totalTbsDl":47204,
     *                       "totalTbsUl":98762,
     *                       "macSdusDl":[
     *                           {
     *                               "sduLength":1232,
     *                               "lcid":3
     *                           }
     *                       ],
     *                       "harqRound":8
     *                   }
     *               },
     *               "harq":[
     *                   "ACK",
     *                   "ACK",
     *                   "ACK",
     *                   "ACK",
     *                   "ACK",
     *                   "ACK",
     *                   "ACK",
     *                   "ACK"
     *               ]
     *           }
     *       ]
     *   }
     *]
     *
     * @apiError IdNotFound The id not found
     *
     */

}

void flexran::north_api::recorder_calls::start_meas(const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string type = request.hasParam(":type") ? request.param(":type").as<std::string>() : "all";
  uint32_t duration = request.hasParam(":duration") ? request.param(":duration").as<uint32_t>() : 1000;

  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");

  std::string id;
  bool success = json_app->start_meas(duration, type, id);
  if (success) {
    response.setMime(MIME(Text, Plain));
    response.headers().add<Pistache::Http::Header::ContentLength>(id.length());
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

  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");

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

  auto mime = Pistache::Http::Mime::MediaType::fromString("application/octet-string");
  response.setMime(mime);
  response.headers().add<Pistache::Http::Header::ContentLength>(ss.str().length());

  response.send(Pistache::Http::Code::Ok, ss.str());
}

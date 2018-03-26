/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

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

#include "stats_manager_calls.h"

void flexran::north_api::stats_manager_calls::register_calls(Pistache::Rest::Router& router)
{
  Pistache::Rest::Routes::Get(router, "/stats_manager/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_stats, this));

 /**
     * @api {get} /stats_manager/json/:stats_type Get RAN statistics by type.
     * @apiName GetStats
     * @apiGroup Stats
     * @apiParam {string} stats_type available types are : enb_config, mac_stats, all
     * @apiDescription This API gets the RAN config and status for the current TTI. The enb_config API endpoint gets eNB, UE, and LC configurations, and the mac_stats gets the status of eNB and UE at different layers namely PDCP, RLC, MAC, and PHY layer form the FlexRAN controller. If the /json is skipped, the return file will be in a string format.
     * @apiVersion v0.1.0
     * @apiPermission None
     * @apiExample Example usage:
     *     curl -X GET http://127.0.0.1:9999/stats_manager/json/all
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
     * @apiExample Example usage:
     *     curl -X GET http://127.0.0.1:9999/stats_manager/all
     *
     * @apiError TypeNotFound The stats type not found.
     *
     * @apiErrorExample Error-Response:
     *     HTTP/1.1 404 Not Found
     *     {
     *       "error": "Statistics type not found"
     *     }
     */
  Pistache::Rest::Routes::Get(router, "/stats/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats, this));

  Pistache::Rest::Routes::Get(router, "/stats/enb/:id/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_enb, this));
}

void flexran::north_api::stats_manager_calls::obtain_stats(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  const std::string type = request.hasParam(":type") ?
      request.param(":type").as<std::string>() : REQ_TYPE::ALL_STATS;

  std::string resp;
  if (type == REQ_TYPE::ALL_STATS) {
    resp = stats_app->all_stats_to_string();
  } else if (type == REQ_TYPE::ENB_CONFIG) {
    resp = stats_app->all_enb_configs_to_string();
  } else if (type == REQ_TYPE::MAC_STATS) {
    resp = stats_app->all_mac_configs_to_string();
  } else {
    response.send(Pistache::Http::Code::Bad_Request, "invalid statistics type\n");
    return;
  }
  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp);
}

void flexran::north_api::stats_manager_calls::obtain_json_stats(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  const std::string type = request.hasParam(":type") ?
      request.param(":type").as<std::string>() : REQ_TYPE::ALL_STATS;
  std::string resp;
  if (type == REQ_TYPE::ALL_STATS) {
    resp = stats_app->all_stats_to_json_string();
  } else if (type == REQ_TYPE::ENB_CONFIG) {
    resp = stats_app->all_enb_configs_to_json_string();
  } else if (type == REQ_TYPE::MAC_STATS) {
    resp = stats_app->all_mac_configs_to_json_string();
  } else {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid statistics type\"}", MIME(Application, Json));
    return;
  }
  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}

void flexran::north_api::stats_manager_calls::obtain_json_stats_enb(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  const std::string enb_id_s = request.param(":id").as<std::string>();
  uint64_t enb_id;
  bool is_agent_id = false;
  try {
    is_agent_id = parse_enb_agent_id(enb_id_s, enb_id);
  } catch (std::invalid_argument e) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid ID\"}", MIME(Application, Json));
    return;
  }

  const std::string type = request.hasParam(":type") ?
      request.param(":type").as<std::string>() : REQ_TYPE::ALL_STATS;
  std::string resp;
  bool found = false;
  if (type == REQ_TYPE::ALL_STATS) {
    found = is_agent_id ?
        stats_app->stats_by_agent_id_to_json_string(static_cast<int>(enb_id), resp)
      : stats_app->stats_by_enb_id_to_json_string(enb_id, resp);
  } else if (type == REQ_TYPE::ENB_CONFIG) {
    found = is_agent_id ?
        stats_app->enb_configs_by_agent_id_to_json_string(static_cast<int>(enb_id), resp)
      : stats_app->enb_configs_by_enb_id_to_json_string(enb_id, resp);
  } else if (type == REQ_TYPE::MAC_STATS) {
    found = is_agent_id ?
        stats_app->mac_configs_by_agent_id_to_json_string(static_cast<int>(enb_id), resp)
      : stats_app->mac_configs_by_enb_id_to_json_string(enb_id, resp);
  } else {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid statistics type\"}", MIME(Application, Json));
    return;
  }

  if (!found) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid ID\" }", MIME(Application, Json));
    return;
  }

  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}

bool flexran::north_api::stats_manager_calls::parse_enb_agent_id(const std::string& enb_id_s, uint64_t& enb_id)
{
  bool is_agent_id = false;
  if (enb_id_s.length() >= AGENT_ID_LENGTH_LIMIT && enb_id_s.substr(0, 2) == "0x") {
    /* it is a hex -> we assume it is not an RNTI */
    enb_id = std::stoi(enb_id_s, 0, 16);
  } else {
    enb_id = std::stoi(enb_id_s);
    /* if the number is shorter than some characters, we assume it is an
     * agent_id (internal identification of the agents in the controller) */
    is_agent_id = enb_id_s.length () < AGENT_ID_LENGTH_LIMIT;
  }
  return is_agent_id;
}

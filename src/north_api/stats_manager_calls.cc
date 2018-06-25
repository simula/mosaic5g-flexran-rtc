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
  /**
   * @api {get} /stats_manager/:type? Get RAN statistics (human-readable)
   * @apiName GetStatsHumanReadable
   * @apiGroup Stats
   * @apiParam {string=enb_config,mac_stats,all} [type=all] The type of
   * statistics to be returned. The following types are allowed:
   * * `enb_config`: static configuration (for eNB, UE, and LC)
   * * `mac_stats`:  statistics about various eNB layers (PDCP, RLC, MAC)
   * * `all`:        both of the above
   *
   * @apiDescription This API gets the RAN config and status for the current
   * TTI for all eNBs connected to this controller. The output is in a
   * human-readable format. For JSON output, see
   * (#Stats:GetStats).
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats_manager/
   *
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats_manager/enb_config
   *
   * @apiError BadRequest The given stats type is invalid.
   *
   * @apiErrorExample Error-Response:
   *    HTTP/1.1 400 BadRequest
   *    invalid statistics type
   */
  Pistache::Rest::Routes::Get(router, "/stats_manager/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_stats, this));

  /**
   * @api {get} /stats/:type? Get RAN statistics in JSON
   * @apiName GetStats
   * @apiGroup Stats
   * @apiParam {string=enb_config,mac_stats,all} [type=all] The type of
   * statistics to be returned. The following types are allowed:
   * * `enb_config`: static configuration (for eNB, UE, and LC)
   * * `mac_stats`:  statistics about various eNB layers (PDCP, RLC, MAC)
   * * `all`:        both of the above
   *
   * @apiDescription This API gets the RAN config and status for the current
   * TTI for all eNBs connected to this controller. The output is in JSON
   * format. For human-readable output, see (#Stats:GetStatsHumanReadable).
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -X GET http://127.0.0.1:9999/stats/
   * @apiSuccessExample Success-Response for one agent + UE connected, all type:
   *     HTTP/1.1 200 OK
   *     {
   *       "eNB_config": [
   *         {
   *           "agent_id": 0,
   *           "eNBId": 234881024,
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "eNBId": "234881024",
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 0,
   *                 "cellId": 0,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 548,
   *                   "sib1Length": 15,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 50,
   *                 "ulBandwidth": 50,
   *                 "ulCyclicPrefixLength": 0,
   *                 "dlCyclicPrefixLength": 0,
   *                 "antennaPortsCount": 1,
   *                 "duplexMode": 1,
   *                 "subframeAssignment": 0,
   *                 "specialSubframePatterns": 0,
   *                 "prachConfigIndex": 0,
   *                 "prachFreqOffset": 2,
   *                 "raResponseWindowSize": 7,
   *                 "macContentionResolutionTimer": 5,
   *                 "maxHARQMsg3Tx": 0,
   *                 "n1PUCCHAN": 0,
   *                 "deltaPUCCHShift": 1,
   *                 "nRBCqi": 0,
   *                 "srsSubframeConfig": 0,
   *                 "srsBwConfig": 0,
   *                 "srsMacUpPts": 0,
   *                 "enable64QAM": 0,
   *                 "carrierIndex": 0,
   *                 "dlFreq": 2685,
   *                 "ulFreq": 2565,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "sliceConfig": {
   *                   "dl": [
   *                     {
   *                       "id": 0,
   *                       "label": "xMBB",
   *                       "percentage": 100,
   *                       "isolation": false,
   *                       "priority": 10,
   *                       "positionLow": 0,
   *                       "positionHigh": 25,
   *                       "maxmcs": 28,
   *                       "sorting": [
   *                         "CR_ROUND",
   *                         "CR_SRB12",
   *                         "CR_HOL",
   *                         "CR_LC",
   *                         "CR_CQI",
   *                         "CR_LCP"
   *                       ],
   *                       "accounting": "POL_FAIR",
   *                       "schedulerName": "schedule_ue_spec"
   *                     }
   *                   ],
   *                   "ul": [
   *                     {
   *                       "id": 0,
   *                       "label": "xMBB",
   *                       "percentage": 100,
   *                       "isolation": false,
   *                       "priority": 0,
   *                       "firstRb": 0,
   *                       "maxmcs": 20,
   *                       "accounting": "POLU_FAIR",
   *                       "schedulerName": "schedule_ulsch_rnti"
   *                     }
   *                   ],
   *                   "intrasliceShareActive": true,
   *                   "intersliceShareActive": true
   *                 }
   *               }
   *             ]
   *           },
   *           "UE": {
   *             "header": {
   *               "version": 0,
   *               "type": 10,
   *               "xid": 0
   *             },
   *             "ueConfig": [
   *               {
   *                 "rnti": 10337,
   *                 "timeAlignmentTimer": 7,
   *                 "transmissionMode": 0,
   *                 "ueAggregatedMaxBitrateUL": "0",
   *                 "ueAggregatedMaxBitrateDL": "0",
   *                 "capabilities": {
   *                   "halfDuplex": 0,
   *                   "intraSFHopping": 0,
   *                   "type2Sb1": 1,
   *                   "ueCategory": 4,
   *                   "resAllocType1": 1
   *                 },
   *                 "ueTransmissionAntenna": 2,
   *                 "ttiBundling": 0,
   *                 "maxHARQTx": 4,
   *                 "betaOffsetACKIndex": 0,
   *                 "betaOffsetRIIndex": 0,
   *                 "betaOffsetCQIIndex": 8,
   *                 "ackNackSimultaneousTrans": 0,
   *                 "simultaneousAckNackCqi": 0,
   *                 "aperiodicCqiRepMode": 3,
   *                 "ackNackRepetitionFactor": 0,
   *                 "pcellCarrierIndex": 0,
   *                 "imsi": "208940100001131",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 2
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 10337,
   *                 "lcConfig": [
   *                   {
   *                     "lcid": 1,
   *                     "lcg": 0,
   *                     "direction": 2,
   *                     "qosBearerType": 0,
   *                     "qci": 1
   *                   },
   *                   {
   *                     "lcid": 2,
   *                     "lcg": 0,
   *                     "direction": 2,
   *                     "qosBearerType": 0,
   *                     "qci": 1
   *                   },
   *                   {
   *                     "lcid": 3,
   *                     "lcg": 1,
   *                     "direction": 1,
   *                     "qosBearerType": 0,
   *                     "qci": 1
   *                   }
   *                 ]
   *               }
   *             ]
   *           }
   *         }
   *       ],
   *       "mac_stats": [
   *         {
   *           "agent_id": 0,
   *           "eNBId": 234881024,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 10337,
   *               "mac_stats": {
   *                 "rnti": 10337,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 31,
   *                 "rlcReport": [
   *                   {
   *                     "lcId": 1,
   *                     "txQueueSize": 0,
   *                     "txQueueHolDelay": 0,
   *                     "statusPduSize": 0
   *                   },
   *                   {
   *                     "lcId": 2,
   *                     "txQueueSize": 0,
   *                     "txQueueHolDelay": 0,
   *                     "statusPduSize": 0
   *                   },
   *                   {
   *                     "lcId": 3,
   *                     "txQueueSize": 0,
   *                     "txQueueHolDelay": 0,
   *                     "statusPduSize": 0
   *                   }
   *                 ],
   *                 "pendingMacCes": 0,
   *                 "dlCqiReport": {
   *                   "sfnSn": 10224,
   *                   "csiReport": [
   *                     {
   *                       "servCellIndex": 0,
   *                       "ri": 0,
   *                       "type": "FLCSIT_P10",
   *                       "p10csi": {
   *                         "wbCqi": 15
   *                       }
   *                     }
   *                   ]
   *                 },
   *                 "ulCqiReport": {
   *                   "sfnSn": 10224,
   *                   "cqiMeas": [
   *                     {
   *                       "type": "FLUCT_SRS",
   *                       "servCellIndex": 0
   *                     }
   *                   ],
   *                   "pucchDbm": [
   *                     {
   *                       "p0PucchDbm": 0,
   *                       "servCellIndex": 0
   *                     }
   *                   ]
   *                 },
   *                 "rrcMeasurements": {
   *                   "measid": -1,
   *                   "pcellRsrp": -1,
   *                   "pcellRsrq": -1
   *                 },
   *                 "pdcpStats": {
   *                   "pktTx": 102,
   *                   "pktTxBytes": 46622,
   *                   "pktTxSn": 101,
   *                   "pktTxW": 1,
   *                   "pktTxBytesW": 52,
   *                   "pktTxAiat": 16557,
   *                   "pktTxAiatW": 101,
   *                   "pktRx": 89,
   *                   "pktRxBytes": 20260,
   *                   "pktRxSn": 99,
   *                   "pktRxW": 1,
   *                   "pktRxBytesW": 90,
   *                   "pktRxAiat": 16512,
   *                   "pktRxAiatW": 28,
   *                   "pktRxOo": 0,
   *                   "sfn": "16627"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 57,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 0,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 3,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 10,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 37962,
   *                   "totalBytesSdusDl": 47290,
   *                   "totalPrbDl": 658,
   *                   "totalPrbUl": 1435,
   *                   "totalPduDl": 74,
   *                   "totalPduUl": 314,
   *                   "totalTbsDl": 49923,
   *                   "totalTbsUl": 47484,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 56,
   *                       "lcid": 3
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 }
   *               },
   *               "harq": [
   *                 "ACK",
   *                 "ACK",
   *                 "ACK",
   *                 "ACK",
   *                 "ACK",
   *                 "ACK",
   *                 "ACK",
   *                 "ACK"
   *               ]
   *             }
   *           ]
   *         }
   *       ]
   *     }
   * @apiError BadRequest The given stats type is invalid.
   *
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "invalid statistics type" }
   */
  Pistache::Rest::Routes::Get(router, "/stats/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats, this));

  /**
   * @api {get} /stats/enb/:id/:type? Get RAN statistics in JSON for one eNB
   * @apiName GetStatsEnb
   * @apiGroup Stats
   * @apiParam {Number} id The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex, preceded by "0x", or decimal) or the internal
   * agent ID which can be obtained through a `stats` call.  Numbers smaller
   * than 1000 are parsed as the agent ID.
   * @apiParam {string=enb_config,mac_stats,all} [type=all] The type of
   * statistics to be returned. The following types are allowed:
   * * `enb_config`: static configuration (for eNB, UE, and LC)
   * * `mac_stats`:  statistics about various eNB layers (PDCP, RLC, MAC)
   * * `all`:        both of the above
   *
   * @apiDescription This API gets the RAN config and status for the current
   * TTI for a given eNB. The output is in JSON format. No human-readable
   * format exists corresponding to this endpoint.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats/enb/-1/
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats/enb/234881037/mac_stats
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats/enb/0xe000000/enb_config
   *
   * @apiError BadRequest The given statistics type or the eNB ID is invalid.
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "invalid statistics type" }
   *
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "invalid ID" }
   */
  Pistache::Rest::Routes::Get(router, "/stats/enb/:id/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_enb, this));

  /**
   * @api {get} /stats/ue/:id/:type? Get UE statistics in JSON
   * @apiName GetStatsUE
   * @apiGroup Stats
   * @apiParam {Number} id The ID of the UE in the form of either an RNTI or
   * the IMSI. Everything shorter than 6 digits will be treated as the RNTI,
   * the rest as the IMSI.
   * @apiParam {string=enb_config,mac_stats,all} [type=all] The type of
   * statistics to be returned. The following types are allowed:
   * * `enb_config`: static configuration (for eNB, UE, and LC)
   * * `mac_stats`:  statistics about various eNB layers (PDCP, RLC, MAC)
   * * `all`:        both of the above
   *
   * @apiDescription This API gets the UE statistics (`mac_stats`) for one UE
   * registered at any eNB managed by the controller. No
   * human-readable format exists corresponding to this endpoint.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -X GET http://127.0.0.1:9999/stats/ue/208940100001131
   *
   * @apiError BadRequest The given eNB ID is invalid.
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "invalid ID" }
   */
  Pistache::Rest::Routes::Get(router, "/stats/ue/:id_ue",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_ue, this));

  /**
   * @api {get} /stats/enb/:id_enb/ue/:id_ue Get UE statistics in JSON, delimited to a given eNB
   * @apiName GetStatsUELimited
   * @apiGroup Stats
   * @apiParam {Number} id_enb The ID of the agent for which to change the
   * slice configuration. This can be one of the following: -1 (last added
   * agent), the eNB ID (in hex, preceded by "0x", or decimal) or the internal
   * agent ID which can be obtained through a `stats` call.  Numbers smaller
   * than 1000 are parsed as the agent ID.
   * @apiParam {number} id_ue The ID of the UE in the form of either an RNTI or
   * the IMSI. Everything shorter than 6 digits will be treated as the RNTI,
   * the rest as the IMSI.
   *
   * @apiDescription This API gets the UE statistics ("mac_stats") for one UE.
   * The search is restrained to a given eNB registered at the controller. No
   * human-readable format exists corresponding to this endpoint.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -X GET http://127.0.0.1:9999/stats/enb/234881037/ue/208940100001131
   *
   * @apiError BadRequest The given eNB ID or UE ID is invalid.
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "invalid ID (eNB and/or UE)" }
   */
  Pistache::Rest::Routes::Get(router, "/stats/enb/:id_enb/ue/:id_ue",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_ue, this));
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
  int agent_id = stats_app->parse_enb_agent_id(request.param(":id").as<std::string>());
  if (agent_id < 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  const std::string type = request.hasParam(":type") ?
      request.param(":type").as<std::string>() : REQ_TYPE::ALL_STATS;

  std::string resp;
  if (type == REQ_TYPE::ALL_STATS) {
    stats_app->stats_by_agent_id_to_json_string(agent_id, resp);
  } else if (type == REQ_TYPE::ENB_CONFIG) {
    stats_app->enb_configs_by_agent_id_to_json_string(agent_id, resp);
  } else if (type == REQ_TYPE::MAC_STATS) {
    stats_app->mac_configs_by_agent_id_to_json_string(agent_id, resp);
  } else {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid statistics type\" }", MIME(Application, Json));
    return;
  }

  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}

void flexran::north_api::stats_manager_calls::obtain_json_stats_ue(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  int agent_id = -1;
  const bool check_enb = request.hasParam(":id_enb");
  if (check_enb) {
    agent_id = stats_app->parse_enb_agent_id(request.param(":id_enb").as<std::string>());
    if (agent_id < 0) {
      response.send(Pistache::Http::Code::Bad_Request,
          "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
      return;
    }
  }


  const std::string ue_id_s = request.param(":id_ue").as<std::string>();
  uint64_t ue_id;
  bool is_rnti = false;
  try {
    is_rnti = parse_ue_id(ue_id_s, ue_id);
  } catch (std::invalid_argument e) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid UE ID\" }", MIME(Application, Json));
    return;
  }

  std::string resp;
  if (check_enb) {
    if (is_rnti)
      stats_app->ue_stats_by_rnti_by_agent_id_to_json_string(
          static_cast<flexran::rib::rnti_t>(ue_id), resp, agent_id);
    else
      /* !is_rnti (==imsi) && agent_id) */
      stats_app->ue_stats_by_imsi_by_agent_id_to_json_string(ue_id, resp, agent_id);
  }
  else {
    if (is_rnti)
      stats_app->ue_stats_by_rnti_to_json_string(static_cast<flexran::rib::rnti_t>(ue_id), resp);
    else
      stats_app->ue_stats_by_imsi_to_json_string(ue_id, resp);
  }

  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}

bool flexran::north_api::stats_manager_calls::parse_ue_id(const std::string& ue_id_s, uint64_t& ue_id)
{
  ue_id = std::stoll(ue_id_s);
  return ue_id_s.length() < RNTI_ID_LENGTH_LIMIT;
}

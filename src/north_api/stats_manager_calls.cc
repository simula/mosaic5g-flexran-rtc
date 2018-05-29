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
   * @apiParam {string} [type=all] available types are: enb_config (eNB
   * configuration), mac_stats (current TTI statistics), all
   *
   * @apiDescription This API gets the RAN config and status for the current TTI
   * for all eNBs in human-readable format. For the type enb_config, the API
   * endpoint gets eNB, UE, and LC configurations, and for mac_stats the status
   * of eNB and UE at different layers, namely PDCP, RLC, MAC, and PHY layer
   * form the FlexRAN controller. For JSON output, see (#Stats:GetStats).
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
   * @apiParam {string} [type=all] available types are: enb_config (eNB
   * configuration), mac_stats (current TTI statistics), all
   *
   * @apiDescription This API gets the RAN config and status for the current TTI
   * for all eNBs in JSON format connected to this Controller. For the type enb_config, 
   * the API endpoint gets eNB, UE, and LC configurations, and for mac_stats the 
   * status of eNB and UE at different layers, namely PDCP, RLC, MAC, and PHY layer 
   * form the FlexRAN controller. For human-readable output, 
   * see (#Stats:GetStatsHumanReadable).
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -X GET http://127.0.0.1:9999/stats/
   * @apiSuccessExample Success-Response:
   *     HTTP/1.1 200 OK
   *     {
   *       "eNB_config": [
   *         {
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "eNBId": "234881037",
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 1,
   *                 "cellId": 0,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 92,
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
   *                 "ulPuschPower": -96
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
   *                 "rnti": 22658,
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
   *                 "imsi": "208940100001131"
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
   *                 "rnti": 22658,
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
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 22658,
   *               "mac_stats": {
   *                 "rnti": 22658,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 40,
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
   *                   "sfnSn": 2902,
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
   *                   "sfnSn": 2902,
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
   *                   "pktTx": 89,
   *                   "pktTxBytes": 33261,
   *                   "pktTxSn": 88,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 19328,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 102,
   *                   "pktRxBytes": 39180,
   *                   "pktRxSn": 101,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 21858,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "22293"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 693,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 0,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 9,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 26,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 59633,
   *                   "totalBytesSdusDl": 33885,
   *                   "totalPrbDl": 526,
   *                   "totalPrbUl": 1748,
   *                   "totalPduDl": 74,
   *                   "totalPduUl": 368,
   *                   "totalTbsDl": 36423,
   *                   "totalTbsUl": 59615,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 655,
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
   *
   * @apiError BadRequest The given stats type is invalid.
   *
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "invalid statistics type" }
   */
  Pistache::Rest::Routes::Get(router, "/stats/:type?",
      Pistache::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_json_stats, this));

  /**
   * @api {get} /stats/enb/:id/:type? Get RAN statistics in JSON
   * @apiName GetStatsEnb
   * @apiGroup Stats
   * @apiParam {string} id the ID of the eNB (agent ID or global eNB ID)
   * @apiParam {string} [type=all] available types are: enb_config (eNB
   * configuration), mac_stats (current TTI statistics), all
   *
   * @apiDescription This API gets the RAN config and status for the current TTI
   * for a given eNB in JSON format. The ID can be either the agent ID (internal
   * ID for every agent in the controller, starting from 0) or the global eNB ID
   * (a mask consisting of the module ID, the eNB ID and the cell ID). The
   * global eNB ID can be in hexadecimal or decimal notation. In the former
   * case, a "0x" needs to preceed the number (hence the ID as string). For the
   * type enb_config, the API endpoint gets eNB, UE, and LC configurations, and
   * for mac_stats the status of eNB and UE at different layers, namely PDCP,
   * RLC, MAC, and PHY layer form the FlexRAN controller. No human-readable
   * format exists corresponding to this endpoint.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats/enb/0/
   * @apiExample Example usage:
   *    curl -X GET http://127.0.0.1:9999/stats/enb/234881037/mac_stats
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
   * @apiParam {number} id the ID of the UE (RNTI or IMSI)
   *
   * @apiDescription This API gets the UE statistics ("mac_stats") for one UE
   * across all UEs registered at eNBs managed by the controller. The ID can be
   * either the RNTI or IMSI which can be obtained via the (#Stats.GetStats)
   * endpoint. No human-readable format exists corresponding to this endpoint.
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
   * @apiParam {number} id_enb the ID of the eNB (agent ID or global eNB ID)
   * @apiParam {number} id_ue the ID of the UE (RNTI or IMSI)
   *
   * @apiDescription This API gets the UE statistics ("mac_stats") for one UE
   * across all UEs. The search is restrained to a given eNB registered at the
   * controller. The ID of the UE can be either the RNTI or IMSI which can be
   * obtained via the (#Stats.GetStats) endpoint. The ID of the eNB can be
   * either the agent ID (internal ID for every agent in the controller, starting
   * from 0) or the global eNB ID (a mask consisting of the module ID, the eNB
   * ID and the cell ID). The global eNB ID can be in hexadecimal or decimal
   * notation. In the former case, a "0x" needs to preceed the number (hence the
   * ID as string). No human-readable format exists corresponding to this
   * endpoint.
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

void flexran::north_api::stats_manager_calls::obtain_json_stats_ue(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  const bool check_enb = request.hasParam(":id_enb");
  uint64_t enb_id;
  bool is_agent_id = false;
  if (check_enb) {
    try {
      is_agent_id = parse_enb_agent_id(request.param(":id_enb").as<std::string>(), enb_id);
    } catch (std::invalid_argument e) {
      response.send(Pistache::Http::Code::Bad_Request,
          "{ \"error\": \"invalid eNB ID\" }", MIME(Application, Json));
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
  bool found = false;
  if (check_enb) {
    if (is_rnti && !is_agent_id)
      found = stats_app->ue_stats_by_rnti_by_enb_id_to_json_string(
          static_cast<flexran::rib::rnti_t>(ue_id), resp, enb_id);
    else if (is_rnti && is_agent_id)
      found = stats_app->ue_stats_by_rnti_by_agent_id_to_json_string(
          static_cast<flexran::rib::rnti_t>(ue_id), resp, static_cast<int>(enb_id));
    else if (!is_rnti && !is_agent_id)
      found = stats_app->ue_stats_by_imsi_by_enb_id_to_json_string(
          ue_id, resp, enb_id);
    else
      /* !is_rnti (==imsi) && agent_id) */
      found = stats_app->ue_stats_by_imsi_by_agent_id_to_json_string(
          ue_id, resp, static_cast<int>(enb_id));
  }
  else {
    found = is_rnti ?
        stats_app->ue_stats_by_rnti_to_json_string(static_cast<flexran::rib::rnti_t>(ue_id), resp)
      : stats_app->ue_stats_by_imsi_to_json_string(ue_id, resp);
  }

  if (!found) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid ID (eNB and/or UE)\" }", MIME(Application, Json));
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
    enb_id = std::stoll(enb_id_s, 0, 16);
  } else {
    enb_id = std::stoll(enb_id_s);
    /* if the number is shorter than some characters, we assume it is an
     * agent_id (internal identification of the agents in the controller) */
    is_agent_id = enb_id_s.length () < AGENT_ID_LENGTH_LIMIT;
  }
  return is_agent_id;
}

bool flexran::north_api::stats_manager_calls::parse_ue_id(const std::string& ue_id_s, uint64_t& ue_id)
{
  ue_id = std::stoll(ue_id_s);
  return ue_id_s.length() < RNTI_ID_LENGTH_LIMIT;
}

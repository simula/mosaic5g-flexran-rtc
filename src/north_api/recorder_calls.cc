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

/*! \file    recorder_calls.cc
 *  \brief   NB API for recorder app
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>
#include <pistache/http_header.h>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>

#include "recorder_calls.h"

void flexran::north_api::recorder_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto recorder = desc.path("/record");

  /**
   * @api {post} /record/[:type/[:duration]]  Record RAN state
   * @apiName postJob
   * @apiGroup Recorder
   * @apiParam {String} type The type specifies the output format. Available
   * types are : all (default), enb, stats, bin (binary)
   * @apiParam {Number{1-}} duration=1000 Duration of record in milliseconds,
   * must be a positive value.
   *
   * @apiDescription This API requests FlexRAN RTC to record the RAN stats of
   * given type for a given duration. Both type and duration parameters are
   * optional. There is only one job at a time possible. Even if the all type
   * is the default, the bin type provides the highest flexibility as the
   * different all other representations (all, enb, stats) can be reproduced
   * using the `parse-bd` utility. Note that the file written by the recorder
   * will be located in the `/tmp/` folder and could be directly copied from
   * there.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage commands:
   *     curl -X POST http://127.0.0.1:9999/record
   *     curl -X POST http://127.0.0.1:9999/record/stats
   *     curl -X POST http://127.0.0.1:9999/record/enb/1
   * @apiSuccess id An identifier through which the recorded data can be
   * recovered.
   * @apiSuccessExample Example success response:
   *     HTTP/1.1 200 OK
   *     { "id" : 123456 }
   * @apiError Conflict A 409 Server State conflict error: Either there is a
   *                    job running or the type or duration were not
   *                    understood.
   * @apiErrorExample Example error response (job already running)
   *    HTTP/1.1 409 Conflict
   *    { "error": "Can not handle request at the moment" }
   */
  recorder.route(desc.post("/:type?/:duration?"),
                 "Record RAN state for a given type and duration")
          .bind(&flexran::north_api::recorder_calls::start_meas, this);


  /**
   * @api {get} /record/:id  Download the recorded RAN state
   * @apiName recoverJob
   * @apiGroup Recorder
   * @apiParam {Number} id Identifier of the previously recorded RAN stats,
   *                       obtained by a successful POST.
   *
   * @apiDescription This API returns the recorded data corresponding to a
   * record job and according to the previously requested type and duration. In
   * the case of the binary type, this call returns a binary representation
   * that can be parsed with the `parse-bd` utility located in `build/`. For
   * more information, execute it without any parameters (`build/parse-bd`).
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -X GET http://127.0.0.1:9999/record/1
   * @apiSuccessExample Success-Response:
   * HTTP/1.1 200 OK
   *
   * [
   *   {
   *     "eNB_config": [
   *       {
   *         "eNB": {
   *           "header": {
   *             "version": 0,
   *             "type": 8,
   *             "xid": 0
   *           },
   *           "cellConfig": [
   *             {
   *               "cellId": 0,
   *               "puschHoppingOffset": 0,
   *               "hoppingMode": 0,
   *               "nSb": 1,
   *               "phichResource": 0,
   *               "phichDuration": 0,
   *               "dlBandwidth": 50,
   *               "ulBandwidth": 50,
   *               "ulCyclicPrefixLength": 0,
   *               "dlCyclicPrefixLength": 0,
   *               "antennaPortsCount": 1,
   *               "duplexMode": 1,
   *               "prachConfigIndex": 0,
   *               "prachFreqOffset": 2,
   *               "raResponseWindowSize": 7,
   *               "macContentionResolutionTimer": 5,
   *               "maxHARQMsg3Tx": 0,
   *               "n1PUCCHAN": 0,
   *               "deltaPUCCHShift": 1,
   *               "nRBCqi": 0,
   *               "srsSubframeConfig": 0,
   *               "srsBwConfig": 0,
   *               "srsMacUpPts": 0,
   *               "enable64QAM": 0,
   *               "carrierIndex": 0,
   *               "dlFreq": 2685,
   *               "ulFreq": 2565,
   *               "eutraBand": 7,
   *               "dlPdschPower": -27,
   *               "ulPuschPower": -96
   *             }
   *           ]
   *         },
   *         "UE": {
   *           "header": {
   *             "version": 0,
   *             "type": 10,
   *             "xid": 0
   *           },
   *           "ueConfig": [
   *             {
   *               "rnti": 28274,
   *               "timeAlignmentTimer": 7,
   *               "transmissionMode": 0,
   *               "ueAggregatedMaxBitrateUL": "0",
   *               "ueAggregatedMaxBitrateDL": "0",
   *               "capabilities": {
   *                 "halfDuplex": 0,
   *                 "intraSFHopping": 0,
   *                 "type2Sb1": 1,
   *                 "ueCategory": 4,
   *                 "resAllocType1": 1
   *               },
   *               "ueTransmissionAntenna": 2,
   *               "ttiBundling": 0,
   *               "maxHARQTx": 4,
   *               "betaOffsetACKIndex": 0,
   *               "betaOffsetRIIndex": 0,
   *               "betaOffsetCQIIndex": 8,
   *               "ackNackSimultaneousTrans": 0,
   *               "simultaneousAckNackCqi": 0,
   *               "aperiodicCqiRepMode": 3,
   *               "ackNackRepetitionFactor": 0,
   *               "pcellCarrierIndex": 0,
   *               "imsi": "208940100001131"
   *             }
   *           ]
   *         },
   *         "LC": {
   *           "header": {
   *             "version": 0,
   *             "type": 12,
   *             "xid": 2
   *           },
   *           "lcUeConfig": [
   *             {
   *               "rnti": 28274,
   *               "lcConfig": [
   *                 {
   *                   "lcid": 1,
   *                   "lcg": 0,
   *                   "direction": 2,
   *                   "qosBearerType": 0,
   *                   "qci": 1
   *                 },
   *                 {
   *                   "lcid": 2,
   *                   "lcg": 0,
   *                   "direction": 2,
   *                   "qosBearerType": 0,
   *                   "qci": 1
   *                 },
   *                 {
   *                   "lcid": 3,
   *                   "lcg": 1,
   *                   "direction": 1,
   *                   "qosBearerType": 0,
   *                   "qci": 1
   *                 }
   *               ]
   *             }
   *           ]
   *         }
   *       }
   *     ],
   *     "mac_stats": [
   *       {
   *         "agent_id": 0,
   *         "ue_mac_stats": [
   *           {
   *             "rnti": 28274,
   *             "mac_stats": {
   *               "rnti": 28274,
   *               "bsr": [
   *                 0,
   *                 0,
   *                 0,
   *                 0
   *               ],
   *               "phr": 40,
   *               "rlcReport": [
   *                 {
   *                   "lcId": 1,
   *                   "txQueueSize": 0,
   *                   "txQueueHolDelay": 0,
   *                   "statusPduSize": 0
   *                 },
   *                 {
   *                   "lcId": 2,
   *                   "txQueueSize": 0,
   *                   "txQueueHolDelay": 0,
   *                   "statusPduSize": 0
   *                 },
   *                 {
   *                   "lcId": 3,
   *                   "txQueueSize": 0,
   *                   "txQueueHolDelay": 0,
   *                   "statusPduSize": 0
   *                 }
   *               ],
   *               "pendingMacCes": 0,
   *               "dlCqiReport": {
   *                 "sfnSn": 15925,
   *                 "csiReport": [
   *                   {
   *                     "servCellIndex": 0,
   *                     "ri": 0,
   *                     "type": "FLCSIT_P10",
   *                     "p10csi": {
   *                       "wbCqi": 15
   *                     }
   *                   }
   *                 ]
   *               },
   *               "ulCqiReport": {
   *                 "sfnSn": 15925,
   *                 "cqiMeas": [
   *                   {
   *                     "type": "FLUCT_SRS",
   *                     "servCellIndex": 0
   *                   }
   *                 ],
   *                 "pucchDbm": [
   *                   {
   *                     "p0PucchDbm": 0,
   *                     "servCellIndex": 0
   *                   }
   *                 ]
   *               },
   *               "rrcMeasurements": {
   *                 "measid": -1,
   *                 "pcellRsrp": -1,
   *                 "pcellRsrq": -1
   *               },
   *               "pdcpStats": {
   *                 "pktTx": 107,
   *                 "pktTxBytes": 26714,
   *                 "pktTxSn": 106,
   *                 "pktTxW": 0,
   *                 "pktTxBytesW": 0,
   *                 "pktTxAiat": 9179,
   *                 "pktTxAiatW": 0,
   *                 "pktRx": 136,
   *                 "pktRxBytes": 80979,
   *                 "pktRxSn": 135,
   *                 "pktRxW": 0,
   *                 "pktRxBytesW": 0,
   *                 "pktRxAiat": 9248,
   *                 "pktRxAiatW": 0,
   *                 "pktRxOo": 0,
   *                 "sfn": "9952"
   *               },
   *               "macStats": {
   *                 "tbsDl": 1143,
   *                 "tbsUl": 63,
   *                 "prbRetxDl": 0,
   *                 "prbRetxUl": 0,
   *                 "prbDl": 15,
   *                 "prbUl": 0,
   *                 "mcs1Dl": 28,
   *                 "mcs2Dl": 26,
   *                 "mcs1Ul": 10,
   *                 "mcs2Ul": 10,
   *                 "totalBytesSdusUl": 99870,
   *                 "totalBytesSdusDl": 27414,
   *                 "totalPrbDl": 490,
   *                 "totalPrbUl": 2312,
   *                 "totalPduDl": 94,
   *                 "totalPduUl": 281,
   *                 "totalTbsDl": 29182,
   *                 "totalTbsUl": 99852,
   *                 "macSdusDl": [
   *                   {
   *                     "sduLength": 1130,
   *                     "lcid": 3
   *                   }
   *                 ],
   *                 "harqRound": 8
   *               }
   *             },
   *             "harq": [
   *               "ACK",
   *               "ACK",
   *               "ACK",
   *               "ACK",
   *               "ACK",
   *               "ACK",
   *               "ACK",
   *               "ACK"
   *             ]
   *           }
   *         ]
   *       }
   *     ]
   *   }
   * ]
   *
   * @apiError BadRequest The ID has not been found or the corresponding file
   *                      could not be opened.
   * @apiErrorExample Example error response (no corresponding job)
   *    HTTP/1.1 400 BadRequest
   *    { "error": "Invalid ID (no such job)" }
   */
  recorder.route(desc.get("/:id"),
                 "Return the recorded data corresponding to a record job")
          .bind(&flexran::north_api::recorder_calls::obtain_json_stats, this);
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
    response.send(Pistache::Http::Code::Ok, "{\"id\":" + id + "}");
  } else {
    /* 409 Bad Conflict -> Server state conflict -> can not handle now */
    response.send(Pistache::Http::Code::Conflict, "{\"error\":\"Can not handle request at the moment\"}");
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
    response.send(Pistache::Http::Code::Bad_Request, "{\"error\":\"Invalid ID (no such job)\"}");
    return;
  }

  std::ifstream file;

  if (info.type == flexran::app::log::job_type::bin)
    file.open(info.filename, std::ios::binary);
  else
    file.open(info.filename);

  if (!file.is_open()) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{\"error\":\"Corresponding file " + info.filename + " could not be opened\"}");
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

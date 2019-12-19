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

/*! \file    stats_manager_calls.cc
 *  \brief   NB API for statistics information
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#include <pistache/http.h>
#include <pistache/http_header.h>

#include "stats_manager_calls.h"

void flexran::north_api::stats_manager_calls::register_calls(Pistache::Rest::Description& desc)
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
   * <a href="#api-Stats-GetStats">Stats:GetStats</a>.
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
  desc.route(desc.get("/stats_manager/:type?"),
                  "Get human-readable RAN config")
      .bind(&flexran::north_api::stats_manager_calls::obtain_stats, this);

  auto stats = desc.path("/stats");

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
   * format. For human-readable output, see <a
   * href="#api-Stats-GetStatsHumanReadable">Stats:GetStatsHumanReadable</a>.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -X GET http://127.0.0.1:9999/stats/
   * @apiSuccessExample Two BSs, X2 en., 2 UEs
   *     HTTP/1.1 200 OK
   *     {
   *       "date_time": "2019-12-19T11:45:44.089",
   *       "eNB_config": [
   *         {
   *           "bs_id": 98765,
   *           "agent_info": [
   *             {
   *               "agent_id": 9,
   *               "ip_port": "192.168.12.161:45756",
   *               "bs_id": 98765,
   *               "capabilities": [
   *                 "LOPHY",
   *                 "HIPHY",
   *                 "LOMAC",
   *                 "HIMAC",
   *                 "RLC",
   *                 "PDCP",
   *                 "SDAP",
   *                 "RRC",
   *                 "S1AP"
   *               ],
   *               "splits": []
   *             }
   *           ],
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 20,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 986,
   *                   "sib1Length": 17,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 25,
   *                 "ulBandwidth": 25,
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
   *                 "dlFreq": 2665,
   *                 "ulFreq": 2545,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "plmnId": [
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 93,
   *                     "mncLength": 2
   *                   }
   *                 ],
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
   *                 },
   *                 "x2HoNetControl": true
   *               }
   *             ],
   *             "s1ap": {
   *               "pending": 0,
   *               "connected": 1,
   *               "enbS1Ip": "192.168.12.161",
   *               "enbName": "eNB-Eurecom-LTEBox",
   *               "mme": [
   *                 {
   *                   "s1Ip": "192.168.12.240",
   *                   "name": "MME 1",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 92,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 93,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 94,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 1,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 93,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 50
   *                 }
   *               ]
   *             }
   *           },
   *           "UE": {
   *             "ueConfig": [
   *               {
   *                 "rnti": 29646,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "208930000000003",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0",
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 0
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 29646,
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
   *         },
   *         {
   *           "bs_id": 123456,
   *           "agent_info": [
   *             {
   *               "agent_id": 10,
   *               "ip_port": "192.168.12.97:46046",
   *               "bs_id": 123456,
   *               "capabilities": [
   *                 "LOPHY",
   *                 "HIPHY",
   *                 "LOMAC",
   *                 "HIMAC",
   *                 "RLC",
   *                 "PDCP",
   *                 "SDAP",
   *                 "RRC",
   *                 "S1AP"
   *               ],
   *               "splits": []
   *             }
   *           ],
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 10,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 842,
   *                   "sib1Length": 17,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 25,
   *                 "ulBandwidth": 25,
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
   *                 "dlFreq": 2665,
   *                 "ulFreq": 2545,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "plmnId": [
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 93,
   *                     "mncLength": 2
   *                   }
   *                 ],
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
   *                 },
   *                 "x2HoNetControl": true
   *               }
   *             ],
   *             "s1ap": {
   *               "pending": 0,
   *               "connected": 1,
   *               "enbS1Ip": "192.168.12.97",
   *               "enbName": "eNB-Eurecom-LTEBox",
   *               "mme": [
   *                 {
   *                   "s1Ip": "192.168.12.240",
   *                   "name": "MME 1",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 92,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 93,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 94,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 1,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 93,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 50
   *                 }
   *               ]
   *             }
   *           },
   *           "UE": {
   *             "ueConfig": [
   *               {
   *                 "rnti": 19349,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "0",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0",
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 0
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 19349,
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
   *           "bs_id": 98765,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 29646,
   *               "mac_stats": {
   *                 "rnti": 29646,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 37,
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
   *                   "sfnSn": 44,
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
   *                   "sfnSn": 44,
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
   *                   "measid": 5,
   *                   "pcellRsrp": -90,
   *                   "pcellRsrq": -2,
   *                   "neighMeas": {
   *                     "eutraMeas": [
   *                       {
   *                         "physCellId": 10,
   *                         "measResult": {
   *                           "rsrp": -101,
   *                           "rsrq": -12
   *                         }
   *                       },
   *                       {
   *                         "physCellId": 237,
   *                         "measResult": {
   *                           "rsrp": -113,
   *                           "rsrq": -19
   *                         }
   *                       }
   *                     ]
   *                   }
   *                 },
   *                 "pdcpStats": {
   *                   "pktTx": 33,
   *                   "pktTxBytes": 9224,
   *                   "pktTxSn": 32,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 91005,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 53,
   *                   "pktRxBytes": 5232,
   *                   "pktRxSn": 53,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 91976,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "92201"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 4,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 4,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 0,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 22149,
   *                   "totalBytesSdusDl": 10488,
   *                   "totalPrbDl": 985,
   *                   "totalPrbUl": 6107,
   *                   "totalPduDl": 442,
   *                   "totalPduUl": 1968,
   *                   "totalTbsDl": 11920,
   *                   "totalTbsUl": 129767,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 2,
   *                       "lcid": 1
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 2375692390,
   *                     "teidSgw": 6
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "192.168.12.240",
   *                   "enbUeS1apId": 7773174,
   *                   "mmeUeS1apId": 553648149,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 93,
   *                     "mncLength": 2
   *                   }
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
   *         },
   *         {
   *           "bs_id": 123456,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 19349,
   *               "mac_stats": {
   *                 "rnti": 19349,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 19,
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
   *                   "sfnSn": 8792,
   *                   "csiReport": [
   *                     {
   *                       "servCellIndex": 0,
   *                       "ri": 0,
   *                       "type": "FLCSIT_P10",
   *                       "p10csi": {
   *                         "wbCqi": 8
   *                       }
   *                     }
   *                   ]
   *                 },
   *                 "ulCqiReport": {
   *                   "sfnSn": 8792,
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
   *                   "measid": 4,
   *                   "pcellRsrp": -119,
   *                   "pcellRsrq": -8,
   *                   "neighMeas": {
   *                     "eutraMeas": [
   *                       {
   *                         "physCellId": 20,
   *                         "measResult": {
   *                           "rsrp": -114,
   *                           "rsrq": -11
   *                         }
   *                       },
   *                       {
   *                         "physCellId": 237,
   *                         "measResult": {
   *                           "rsrp": -126,
   *                           "rsrq": -19
   *                         }
   *                       }
   *                     ]
   *                   }
   *                 },
   *                 "pdcpStats": {
   *                   "pktTx": 30,
   *                   "pktTxBytes": 9961,
   *                   "pktTxSn": 29,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 88986,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 36,
   *                   "pktRxBytes": 9848,
   *                   "pktRxSn": 39,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 88976,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "90709"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 4,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 2,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 13,
   *                   "mcs2Dl": 0,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 16810,
   *                   "totalBytesSdusDl": 10917,
   *                   "totalPrbDl": 780,
   *                   "totalPrbUl": 1847,
   *                   "totalPduDl": 223,
   *                   "totalPduUl": 537,
   *                   "totalTbsDl": 11575,
   *                   "totalTbsUl": 44716,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 2,
   *                       "lcid": 1
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 3396329693,
   *                     "teidSgw": 7
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "192.168.12.240",
   *                   "enbUeS1apId": 16398672,
   *                   "mmeUeS1apId": 553648150,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 93,
   *                     "mncLength": 2
   *                   }
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
   * @apiSuccessExample Monol. BS, S1-flex, 2 UEs
   *     HTTP/1.1 200 OK
   *     {
   *       "date_time": "2019-12-18T15:45:20.882",
   *       "eNB_config": [
   *         {
   *           "bs_id": 123456,
   *           "agent_info": [
   *             {
   *               "agent_id": 5,
   *               "ip_port": "127.0.0.1:56438",
   *               "bs_id": 123456,
   *               "capabilities": [
   *                 "LOPHY",
   *                 "HIPHY",
   *                 "LOMAC",
   *                 "HIMAC",
   *                 "RLC",
   *                 "PDCP",
   *                 "SDAP",
   *                 "RRC",
   *                 "S1AP"
   *               ],
   *               "splits": []
   *             }
   *           ],
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 0,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 762,
   *                   "sib1Length": 20,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 25,
   *                 "ulBandwidth": 25,
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
   *                 "dlFreq": 2665,
   *                 "ulFreq": 2545,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "plmnId": [
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   },
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 94,
   *                     "mncLength": 2
   *                   }
   *                 ],
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
   *                 },
   *                 "x2HoNetControl": false
   *               }
   *             ],
   *             "s1ap": {
   *               "pending": 0,
   *               "connected": 2,
   *               "enbS1Ip": "192.168.12.45",
   *               "enbName": "eNB-Eurecom-LTEBox",
   *               "mme": [
   *                 {
   *                   "s1Ip": "192.168.12.119",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 4,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 95,
   *                       "mncLength": 2
   *                     },
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 94,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 10
   *                 },
   *                 {
   *                   "s1Ip": "192.168.12.170",
   *                   "name": "MME 1",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 92,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 93,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 94,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     },
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 32768,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 95,
   *                       "mncLength": 2
   *                     },
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 94,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 50
   *                 }
   *               ]
   *             }
   *           },
   *           "UE": {
   *             "ueConfig": [
   *               {
   *                 "rnti": 53123,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "0",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               },
   *               {
   *                 "rnti": 12471,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "208950000000011",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 0
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 53123,
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
   *               },
   *               {
   *                 "rnti": 12471,
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
   *           "bs_id": 123456,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 12471,
   *               "mac_stats": {
   *                 "rnti": 12471,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 36,
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
   *                   "sfnSn": 7809,
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
   *                   "sfnSn": 7809,
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
   *                   "measid": 2,
   *                   "pcellRsrp": -87,
   *                   "pcellRsrq": -2
   *                 },
   *                 "pdcpStats": {
   *                   "pktTx": 59,
   *                   "pktTxBytes": 15185,
   *                   "pktTxSn": 58,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 128541,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 74,
   *                   "pktRxBytes": 10406,
   *                   "pktRxSn": 76,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 128569,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "140926"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 4,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 0,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 0,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 20085,
   *                   "totalBytesSdusDl": 16481,
   *                   "totalPrbDl": 1003,
   *                   "totalPrbUl": 5410,
   *                   "totalPduDl": 435,
   *                   "totalPduUl": 1754,
   *                   "totalTbsDl": 17886,
   *                   "totalTbsUl": 117184,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 2,
   *                       "lcid": 1
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 2375692390,
   *                     "teidSgw": 5
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "192.168.12.119",
   *                   "enbUeS1apId": 7773174,
   *                   "mmeUeS1apId": 5,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
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
   *             },
   *             {
   *               "rnti": 53123,
   *               "mac_stats": {
   *                 "rnti": 53123,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 32,
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
   *                   "sfnSn": 7809,
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
   *                   "sfnSn": 7809,
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
   *                   "measid": 2,
   *                   "pcellRsrp": -89,
   *                   "pcellRsrq": -2
   *                 },
   *                 "pdcpStats": {
   *                   "pktTx": 5334,
   *                   "pktTxBytes": 3363877,
   *                   "pktTxSn": 1237,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 123211,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 2500,
   *                   "pktRxBytes": 647681,
   *                   "pktRxSn": 2538,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 140040,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "140926"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 4,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 2,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 0,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 684324,
   *                   "totalBytesSdusDl": 3387478,
   *                   "totalPrbDl": 40133,
   *                   "totalPrbUl": 21913,
   *                   "totalPduDl": 2884,
   *                   "totalPduUl": 4134,
   *                   "totalTbsDl": 3422958,
   *                   "totalTbsUl": 854645,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 2,
   *                       "lcid": 1
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 2369645879,
   *                     "teidSgw": 893
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "192.168.12.170",
   *                   "enbUeS1apId": 16398672,
   *                   "mmeUeS1apId": 687865978,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 94,
   *                     "mncLength": 2
   *                   }
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
   * @apiSuccessExample RCC+RRU, 1 UE
   *     HTTP/1.1 200 OK
   *     {
   *       "date_time": "2019-12-18T16:20:44.124",
   *       "eNB_config": [
   *         {
   *           "bs_id": 10006,
   *           "agent_info": [
   *             {
   *               "agent_id": 6,
   *               "ip_port": "127.0.0.1:56778",
   *               "bs_id": 10006,
   *               "capabilities": [
   *                 "LOPHY",
   *                 "HIPHY",
   *                 "LOMAC",
   *                 "HIMAC",
   *                 "RLC",
   *                 "PDCP",
   *                 "SDAP",
   *                 "RRC",
   *                 "S1AP"
   *               ],
   *               "splits": [
   *                 "IF4p5"
   *               ]
   *             }
   *           ],
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 0,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 40,
   *                   "sib1Length": 17,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 25,
   *                 "ulBandwidth": 25,
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
   *                 "dlFreq": 2665,
   *                 "ulFreq": 2545,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "plmnId": [
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
   *                 ],
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
   *                 },
   *                 "x2HoNetControl": false
   *               }
   *             ],
   *             "s1ap": {
   *               "pending": 0,
   *               "connected": 1,
   *               "enbS1Ip": "127.0.1.30",
   *               "enbName": "eNB-Eurecom-LTEBox",
   *               "mme": [
   *                 {
   *                   "s1Ip": "127.0.1.10",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 4,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 95,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 10
   *                 }
   *               ]
   *             }
   *           },
   *           "UE": {
   *             "ueConfig": [
   *               {
   *                 "rnti": 15184,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "208950000000011",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 0
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 15184,
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
   *           "bs_id": 10006,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 15184,
   *               "mac_stats": {
   *                 "rnti": 15184,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 32,
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
   *                   "sfnSn": 1264,
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
   *                   "sfnSn": 1264,
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
   *                   "pktTx": 19,
   *                   "pktTxBytes": 2511,
   *                   "pktTxSn": 18,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 8904,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 26,
   *                   "pktRxBytes": 3205,
   *                   "pktRxSn": 25,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 9356,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "11501"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 61,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 0,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 14,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 4273,
   *                   "totalBytesSdusDl": 2850,
   *                   "totalPrbDl": 80,
   *                   "totalPrbUl": 775,
   *                   "totalPduDl": 31,
   *                   "totalPduUl": 245,
   *                   "totalTbsDl": 3009,
   *                   "totalTbsUl": 17704,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 56,
   *                       "lcid": 3
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 3396329693,
   *                     "teidSgw": 2
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "127.0.1.10",
   *                   "enbUeS1apId": 420141,
   *                   "mmeUeS1apId": 2,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
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
   * @apiSuccessExample DU+CU, 1 UE
   *     HTTP/1.1 200 OK
   *     {
   *       "date_time": "2019-12-18T16:58:34.778",
   *       "eNB_config": [
   *         {
   *           "bs_id": 12345678,
   *           "agent_info": [
   *             {
   *               "agent_id": 2,
   *               "ip_port": "192.168.12.119:37158",
   *               "bs_id": 12345678,
   *               "capabilities": [
   *                 "PDCP",
   *                 "SDAP",
   *                 "RRC",
   *                 "S1AP"
   *               ],
   *               "splits": [
   *                 "F1"
   *               ]
   *             },
   *             {
   *               "agent_id": 3,
   *               "ip_port": "127.0.0.1:59178",
   *               "bs_id": 12345678,
   *               "capabilities": [
   *                 "LOPHY",
   *                 "HIPHY",
   *                 "LOMAC",
   *                 "HIMAC",
   *                 "RLC"
   *               ],
   *               "splits": [
   *                 "F1"
   *               ]
   *             }
   *           ],
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 0,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 514,
   *                   "sib1Length": 17,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 25,
   *                 "ulBandwidth": 25,
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
   *                 "dlFreq": 2665,
   *                 "ulFreq": 2545,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "plmnId": [
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
   *                 ],
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
   *                 },
   *                 "x2HoNetControl": false
   *               }
   *             ],
   *             "s1ap": {
   *               "pending": 0,
   *               "connected": 1,
   *               "enbS1Ip": "192.168.12.119",
   *               "enbName": "eNB-CU-Eurecom-LTEBox",
   *               "mme": [
   *                 {
   *                   "s1Ip": "192.168.12.45",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 4,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 95,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 10
   *                 }
   *               ]
   *             }
   *           },
   *           "UE": {
   *             "ueConfig": [
   *               {
   *                 "rnti": 28189,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "208950000000011",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 0
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 28189,
   *                 "lcConfig": [
   *                   {
   *                     "lcid": 1,
   *                     "lcg": 0,
   *                     "direction": 2,
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
   *           "bs_id": 12345678,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 28189,
   *               "mac_stats": {
   *                 "rnti": 28189,
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
   *                   "sfnSn": 5506,
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
   *                   "sfnSn": 5506,
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
   *                   "pktTx": 223,
   *                   "pktTxBytes": 218709,
   *                   "pktTxSn": 222,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 7,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 238,
   *                   "pktRxBytes": 48063,
   *                   "pktRxSn": 237,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 7,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "7"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 61,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 0,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 14,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 50069,
   *                   "totalBytesSdusDl": 219962,
   *                   "totalPrbDl": 2514,
   *                   "totalPrbUl": 1305,
   *                   "totalPduDl": 191,
   *                   "totalPduUl": 189,
   *                   "totalTbsDl": 223687,
   *                   "totalTbsUl": 56882,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 56,
   *                       "lcid": 3
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 3396329693,
   *                     "teidSgw": 1
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "192.168.12.45",
   *                   "enbUeS1apId": 420141,
   *                   "mmeUeS1apId": 1,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
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
   * @apiSuccessExample RU+DU+CU, 1 UE
   *     HTTP/1.1 200 OK
   *     {
   *       "date_time": "2019-12-18T17:02:21.178",
   *       "eNB_config": [
   *         {
   *           "bs_id": 12345678,
   *           "agent_info": [
   *             {
   *               "agent_id": 6,
   *               "ip_port": "127.0.0.1:59568",
   *               "bs_id": 12345678,
   *               "capabilities": [
   *                 "LOPHY",
   *                 "HIPHY",
   *                 "LOMAC",
   *                 "HIMAC",
   *                 "RLC"
   *               ],
   *               "splits": [
   *                 "F1",
   *                 "IF4p5"
   *               ]
   *             },
   *             {
   *               "agent_id": 5,
   *               "ip_port": "192.168.12.119:37160",
   *               "bs_id": 12345678,
   *               "capabilities": [
   *                 "PDCP",
   *                 "SDAP",
   *                 "RRC",
   *                 "S1AP"
   *               ],
   *               "splits": [
   *                 "F1"
   *               ]
   *             }
   *           ],
   *           "eNB": {
   *             "header": {
   *               "version": 0,
   *               "type": 8,
   *               "xid": 0
   *             },
   *             "cellConfig": [
   *               {
   *                 "phyCellId": 0,
   *                 "puschHoppingOffset": 0,
   *                 "hoppingMode": 0,
   *                 "nSb": 1,
   *                 "phichResource": 0,
   *                 "phichDuration": 0,
   *                 "initNrPDCCHOFDMSym": 1,
   *                 "siConfig": {
   *                   "sfn": 595,
   *                   "sib1Length": 17,
   *                   "siWindowLength": 5
   *                 },
   *                 "dlBandwidth": 25,
   *                 "ulBandwidth": 25,
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
   *                 "dlFreq": 2665,
   *                 "ulFreq": 2545,
   *                 "eutraBand": 7,
   *                 "dlPdschPower": -27,
   *                 "ulPuschPower": -96,
   *                 "plmnId": [
   *                   {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
   *                 ],
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
   *                 },
   *                 "x2HoNetControl": false
   *               }
   *             ],
   *             "s1ap": {
   *               "pending": 0,
   *               "connected": 1,
   *               "enbS1Ip": "192.168.12.119",
   *               "enbName": "eNB-CU-Eurecom-LTEBox",
   *               "mme": [
   *                 {
   *                   "s1Ip": "192.168.12.45",
   *                   "state": "FLMMES_CONNECTED",
   *                   "servedGummeis": [
   *                     {
   *                       "plmn": {
   *                         "mcc": 208,
   *                         "mnc": 95,
   *                         "mncLength": 2
   *                       },
   *                       "mmeGroupId": 4,
   *                       "mmeCode": 1
   *                     }
   *                   ],
   *                   "requestedPlmns": [
   *                     {
   *                       "mcc": 208,
   *                       "mnc": 95,
   *                       "mncLength": 2
   *                     }
   *                   ],
   *                   "relCapacity": 10
   *                 }
   *               ]
   *             }
   *           },
   *           "UE": {
   *             "ueConfig": [
   *               {
   *                 "rnti": 29211,
   *                 "timeAlignmentTimer": 7,
   *                 "measGapConfigPattern": 4294967295,
   *                 "measGapConfigSfOffset": 4294967295,
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
   *                 "tddAckNackFeedback": 4294967295,
   *                 "ackNackRepetitionFactor": 0,
   *                 "extendedBsrSize": 4294967295,
   *                 "imsi": "208950000000011",
   *                 "dlSliceId": 0,
   *                 "ulSliceId": 0,
   *                 "info": {
   *                   "offsetFreqServing": "0",
   *                   "offsetFreqNeighbouring": "0",
   *                   "cellIndividualOffset": [
   *                     "0"
   *                   ],
   *                   "filterCoefficientRsrp": "4",
   *                   "filterCoefficientRsrq": "4",
   *                   "event": {
   *                     "a3": {
   *                       "a3Offset": "0",
   *                       "reportOnLeave": 1,
   *                       "hysteresis": "0",
   *                       "timeToTrigger": "40",
   *                       "maxReportCells": "2"
   *                     }
   *                   }
   *                 }
   *               }
   *             ]
   *           },
   *           "LC": {
   *             "header": {
   *               "version": 0,
   *               "type": 12,
   *               "xid": 0
   *             },
   *             "lcUeConfig": [
   *               {
   *                 "rnti": 29211,
   *                 "lcConfig": [
   *                   {
   *                     "lcid": 1,
   *                     "lcg": 0,
   *                     "direction": 2,
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
   *           "bs_id": 12345678,
   *           "ue_mac_stats": [
   *             {
   *               "rnti": 29211,
   *               "mac_stats": {
   *                 "rnti": 29211,
   *                 "bsr": [
   *                   0,
   *                   0,
   *                   0,
   *                   0
   *                 ],
   *                 "phr": 21,
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
   *                   "sfnSn": 6694,
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
   *                   "sfnSn": 6694,
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
   *                   "pktTx": 361,
   *                   "pktTxBytes": 309300,
   *                   "pktTxSn": 360,
   *                   "pktTxW": 0,
   *                   "pktTxBytesW": 0,
   *                   "pktTxAiat": 7,
   *                   "pktTxAiatW": 0,
   *                   "pktRx": 289,
   *                   "pktRxBytes": 107401,
   *                   "pktRxSn": 288,
   *                   "pktRxW": 0,
   *                   "pktRxBytesW": 0,
   *                   "pktRxAiat": 7,
   *                   "pktRxAiatW": 0,
   *                   "pktRxOo": 0,
   *                   "sfn": "7"
   *                 },
   *                 "macStats": {
   *                   "tbsDl": 61,
   *                   "tbsUl": 63,
   *                   "prbRetxDl": 0,
   *                   "prbRetxUl": 0,
   *                   "prbDl": 2,
   *                   "prbUl": 0,
   *                   "mcs1Dl": 28,
   *                   "mcs2Dl": 14,
   *                   "mcs1Ul": 10,
   *                   "mcs2Ul": 10,
   *                   "totalBytesSdusUl": 109747,
   *                   "totalBytesSdusDl": 249471,
   *                   "totalPrbDl": 2921,
   *                   "totalPrbUl": 2610,
   *                   "totalPduDl": 258,
   *                   "totalPduUl": 477,
   *                   "totalTbsDl": 253259,
   *                   "totalTbsUl": 118745,
   *                   "macSdusDl": [
   *                     {
   *                       "sduLength": 56,
   *                       "lcid": 3
   *                     }
   *                   ],
   *                   "harqRound": 8
   *                 },
   *                 "gtpStats": [
   *                   {
   *                     "eRabId": 5,
   *                     "teidEnb": 3396329693,
   *                     "teidSgw": 2
   *                   }
   *                 ],
   *                 "s1apStats": {
   *                   "mmeS1Ip": "192.168.12.45",
   *                   "enbUeS1apId": 420141,
   *                   "mmeUeS1apId": 2,
   *                   "selectedPlmn": {
   *                     "mcc": 208,
   *                     "mnc": 95,
   *                     "mncLength": 2
   *                   }
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
  stats.route(desc.get("/:type?"),
              "Get JSON RAN config for type")
       .bind(&flexran::north_api::stats_manager_calls::obtain_json_stats, this);

  /**
   * @api {get} /stats/enb/:id/:type? Get RAN statistics in JSON for one eNB
   * @apiName GetStatsEnb
   * @apiGroup Stats
   * @apiParam {Number} id The ID of the desired BS. This can be one of the
   * following: -1 (last added agent), the eNB ID (in hex, preceded by "0x", or
   * decimal) or the internal agent ID which can be obtained through a `stats`
   * call.  Numbers smaller than 1000 are parsed as the agent ID.
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
  stats.route(desc.get("/enb/:id/:type?"),
              "Get JSON RAN config for BS and type")
       .bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_enb, this);

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
  stats.route(desc.get("/ue/:id_ue"),
              "Gets UE statistics for a given UE")
       .bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_ue, this);

  /**
   * @api {get} /stats/enb/:id_enb/ue/:id_ue Get UE statistics in JSON, delimited to a given eNB
   * @apiName GetStatsUELimited
   * @apiGroup Stats
   * @apiParam {Number} id_enb The ID of the desired BS. This can be one of the
   * following: -1 (last added agent), the eNB ID (in hex, preceded by "0x", or
   * decimal) or the internal agent ID which can be obtained through a `stats`
   * call.  Numbers smaller than 1000 are parsed as the agent ID.
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
  stats.route(desc.get("/enb/:id_enb/ue/:id_ue"),
              "Get UE statistics for a UE on a BS")
       .bind(&flexran::north_api::stats_manager_calls::obtain_json_stats_ue, this);

  /**
   * @api {get} /stats/conf/enb/:id? Get statistics configuration
   * @apiName GetStatsConf
   * @apiGroup Stats
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the desired BS. This
   * can be one of the following: -1 (last added agent), the eNB ID (in hex,
   * preceded by "0x", or decimal) or the internal agent ID which can be
   * obtained through a `stats` call.  Numbers smaller than 1000 are parsed as
   * the agent ID.
   *
   * @apiDescription This returns the active statistics configuration for a
   * given BS. The configuration governs which statistics are sent how often
   * from the BS to the controller. For a description of the returned
   * parameters, check the <a href="#api-Stats-SetStatsConf">Stats:SetStatsConf</a>
   * API endpoint.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -XGET http://127.0.0.1:9999/stats/conf/enb/
   * @apiSuccessExample Success-Response:
   * {
   *   "reports": [
   *     {
   *       "reportFrequency": "FLSRF_PERIODICAL",
   *       "sf": 1000,
   *       "cellReports": [
   *         "FLCST_NOISE_INTERFERENCE"
   *       ],
   *       "ueReports": [
   *         "FLUST_PHR",
   *         "FLUST_DL_CQI",
   *         "FLUST_BSR",
   *         "FLUST_RLC_BS",
   *         "FLUST_MAC_CE_BS",
   *         "FLUST_UL_CQI",
   *         "FLUST_RRC_MEASUREMENTS",
   *         "FLUST_PDCP_STATS",
   *         "FLUST_MAC_STATS",
   *         "FLUST_GTP_STATS"
   *       ]
   *     }
   *   ]
   * }
   *
   * @apiError BadRequest The given eNB ID is invalid.
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "unknown BS" }
   */
  stats.route(desc.get("/conf/enb/:id?/"),
              "Get the statistics configuration")
       .bind(&flexran::north_api::stats_manager_calls::get_stats_req, this);

  /**
   * @api {post} /stats/conf/enb/:id? Set statistics configuration
   * @apiName SetStatsConf
   * @apiGroup Stats
   * @apiParam (URL parameter) {Number} [id=-1] The ID of the desired BS. This
   * can be one of the following: -1 (last added agent), the eNB ID (in hex,
   * preceded by "0x", or decimal) or the internal agent ID which can be
   * obtained through a `stats` call.  Numbers smaller than 1000 are parsed as
   * the agent ID.
   * @apiParam (JSON parameter) {Object[]} reports An array of individual
   * configurations.
   * @apiParam (JSON parameter) {String="FLSRF_PERIODICAL"} reports[reportFrequency] Report
   * frequency type, only periodical reports are implemented at the moment.
   * @apiParam (JSON parameter) {Number{1-}} reports[sf] The periodicity in subframes
   * (i.e., milliseconds).
   * @apiParam (JSON parameter) {String[]="FLCST_NOISE_INTERFERENCE"} reports[cellReports] The
   * cell-level statistics. Implemented: noise/interference measurements.
   * @apiParam (JSON parameter) {String[]="FLUST_PHR","FLUST_DL_CQI","FLUST_UL_CQI",
   * "FLUST_BSR","FLUST_RLC_BS","FLUST_MAC_CE_BS","FLUST_RRC_MEASUREMENTS",
   * "FLUST_PDCP_STATS","FLUST_MAC_STATS","FLUST_GTP_STATS"}  reports[ueReports] The
   * UE-level (per-UE) reports. Implemented: power headroom, DL/UL CQI, buffer
   * status report, RLC report, pending MAC Control Elements, RRC measurements,
   * PDCP statistics (e.g., packets bytes), MAC statistics (e.g., used PRBs),
   * GTP statistics (used tunnel identifiers).
   *
   * @apiDescription This sets a new statistics configuration for a
   * given BS. The configuration governs which statistics are sent how often
   * from the BS to the controller. The controller will delete all current
   * statistics configurations present in the BS and configure the given ones.
   * To modify an existing configuration, use the <a
   * href="#api-Stats-GetStatsConf">GetStatsConf</a> endpoint:
   * `curl localhost:9999/stats/conf/enb | jq . > stats.json`.
   *
   * @apiVersion v0.1.0
   * @apiPermission None
   * @apiExample Example usage:
   *     curl -XPOST http://127.0.0.1:9999/stats/conf/enb/ --data-binary @stats.json
   *
   * @apiError BadRequest The given eNB ID is invalid.
   * @apiErrorExample Error-Response:
   *     HTTP/1.1 400 BadRequest
   *     { "error": "unknown BS" }
   */
  stats.route(desc.post("/conf/enb/:id?/"),
              "Set the statistics configuration")
       .bind(&flexran::north_api::stats_manager_calls::set_stats_req, this);
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
  uint64_t bs_id = stats_app->parse_bs_agent_id(request.param(":id").as<std::string>());
  if (bs_id == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"can not find agent\" }", MIME(Application, Json));
    return;
  }

  const std::string type = request.hasParam(":type") ?
      request.param(":type").as<std::string>() : REQ_TYPE::ALL_STATS;

  std::string resp;
  if (type == REQ_TYPE::ALL_STATS) {
    stats_app->stats_by_bs_id_to_json_string(bs_id, resp);
  } else if (type == REQ_TYPE::ENB_CONFIG) {
    stats_app->enb_configs_by_bs_id_to_json_string(bs_id, resp);
  } else if (type == REQ_TYPE::MAC_STATS) {
    stats_app->mac_configs_by_bs_id_to_json_string(bs_id, resp);
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
  uint64_t bs_id = 0;
  const bool check_enb = request.hasParam(":id_enb");
  if (check_enb) {
    bs_id = stats_app->parse_bs_agent_id(request.param(":id_enb").as<std::string>());
    if (bs_id == 0) {
      response.send(Pistache::Http::Code::Bad_Request,
          "{ \"error\": \"can not find BS\" }", MIME(Application, Json));
      return;
    }
  }

  const std::string ue_id_s = request.param(":id_ue").as<std::string>();
  flexran::rib::rnti_t rnti;
  bool found = false;
  found = check_enb ? stats_app->parse_rnti_imsi(bs_id, ue_id_s, rnti) :
                      stats_app->parse_rnti_imsi_find_bs(ue_id_s, rnti, bs_id);
  if (!found) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"invalid UE ID\" }", MIME(Application, Json));
    return;
  }

  /* at this point, both the correct bs_id and RNTI will be known */
  std::string resp;
  stats_app->ue_stats_by_rnti_by_bs_id_to_json_string(rnti, resp, bs_id);
  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}

void flexran::north_api::stats_manager_calls::get_stats_req(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  std::string bs = "-1";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();

  std::string resp;
  if (!stats_app->get_stats_requests(bs, resp)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + resp +"\" }", MIME(Application, Json));
    return;
  }

  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}

void flexran::north_api::stats_manager_calls::set_stats_req(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  std::string bs = "-1";
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();

  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  std::string error_reason;
  if (!stats_app->set_stats_requests(bs, policy, error_reason)) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"" + error_reason + "\" }\n", MIME(Application, Json));
    return;
  }
  response.send(Pistache::Http::Code::Ok, "");
}

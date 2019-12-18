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

/*! \file    ue_mac_rib_info.cc
 *  \brief   wrapper class for a UE's configuration
 *  \authors Xenofon Foukas, Navid Nikaein, Shahab SHARIAT BAGHERI, Robert
 *           Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           shahab.shariat@eurecom.fr, robert.schmidt@eurecom.fr
 */

#include <iostream>
#include <sstream>
#include <string>
#include <google/protobuf/util/json_util.h>

#include "ue_mac_rib_info.h"
#include "flexran_log.h"

void flexran::rib::ue_mac_rib_info::update_dl_sf_info(const protocol::flex_dl_info& dl_info) {
  uint8_t CC_id = dl_info.serv_cell_index();
  uint8_t harq_id = dl_info.harq_process_id();
  
  for (int i = 0; i < dl_info.harq_status_size(); i++) {
    LOG4CXX_DEBUG(flog::rib, "HARQ ID " << static_cast<uint16_t>(harq_id)
        << ", HARQ status " << dl_info.harq_status(i) << ", CC "
        << static_cast<uint16_t>(CC_id));
    harq_stats_[CC_id][harq_id][i] = dl_info.harq_status(i);
    active_harq_[CC_id][harq_id][i] = true;
  }
}

void flexran::rib::ue_mac_rib_info::update_ul_sf_info(const protocol::flex_ul_info& ul_info) {
  tpc_ = ul_info.tpc();
  uint8_t CC_id = ul_info.serv_cell_index();
  uplink_reception_stats_[CC_id] = ul_info.reception_status();
  for (int i = 0; i < ul_info.ul_reception_size(); i++) {
    ul_reception_data_[CC_id][i] = ul_info.ul_reception(i);
  }
}

void flexran::rib::ue_mac_rib_info::update_mac_stats_report(const protocol::flex_ue_stats_report& stats_report) {
  // Check the flags of the incoming report and copy only those elements that have been updated
  uint32_t flags = stats_report.flags();

  // lock the mutex for the duration of this method
  std::lock_guard<std::mutex> guard(mac_stats_report_mutex_);

  mac_stats_report_.set_rnti(stats_report.rnti());
  if (protocol::FLUST_BSR & flags) {
    mac_stats_report_.clear_bsr();
    for (int i = 0; i < stats_report.bsr_size(); i++) {
      mac_stats_report_.add_bsr(stats_report.bsr(i));
    }
  }
  if (protocol::FLUST_PHR & flags) {
    mac_stats_report_.set_phr(stats_report.phr());
  }
  if (protocol::FLUST_RLC_BS & flags) {
    mac_stats_report_.mutable_rlc_report()->CopyFrom(stats_report.rlc_report());

    //if (stats_report.rlc_report_size() == mac_stats_report_.rlc_report_size()) {
    //  for (int i = 0; i < mac_stats_report_.rlc_report_size(); i++) {
    //	mac_stats_report_.mutable_rlc_report(i)->CopyFrom(stats_report.rlc_report(i));
    //      }
    //    } else {
    //      mac_stats_report_.clear_rlc_report();
    //      for (int i = 0; i < stats_report.rlc_report_size(); i++) {
    //	mac_stats_report_.add_rlc_report();
    //	mac_stats_report_.mutable_rlc_report(i)->CopyFrom(stats_report.rlc_report(i));
    //      }
    //    }
  }
  if (protocol::FLUST_MAC_CE_BS & flags) {
    mac_stats_report_.set_pending_mac_ces(stats_report.pending_mac_ces());
  }
  if (protocol::FLUST_DL_CQI & flags) {
    mac_stats_report_.mutable_dl_cqi_report()->CopyFrom(stats_report.dl_cqi_report());
  }
  if (protocol::FLUST_PBS & flags) {
    mac_stats_report_.mutable_pbr()->CopyFrom(stats_report.pbr());
  }
  if (protocol::FLUST_UL_CQI & flags) {
    mac_stats_report_.mutable_ul_cqi_report()->CopyFrom(stats_report.ul_cqi_report());
  }

  if (protocol::FLUST_PDCP_STATS & flags) {
   mac_stats_report_.mutable_pdcp_stats()->CopyFrom(stats_report.pdcp_stats());
  }

  if (protocol::FLUST_RRC_MEASUREMENTS & flags) {
   mac_stats_report_.mutable_rrc_measurements()->CopyFrom(stats_report.rrc_measurements());
  }

  if (protocol::FLUST_MAC_STATS & flags) {
   mac_stats_report_.mutable_mac_stats()->CopyFrom(stats_report.mac_stats());
  }

  if (protocol::FLUST_GTP_STATS & flags)
    mac_stats_report_.mutable_gtp_stats()->CopyFrom(stats_report.gtp_stats());

  if (protocol::FLUST_S1AP_STATS & flags)
    mac_stats_report_.mutable_s1ap_stats()->CopyFrom(stats_report.s1ap_stats());
}

void flexran::rib::ue_mac_rib_info::dump_stats() const {
  LOG4CXX_INFO(flog::rib, "Rnti: " << rnti_);
  mac_stats_report_mutex_.lock();
  LOG4CXX_INFO(flog::rib, mac_stats_report_.DebugString());
  mac_stats_report_mutex_.unlock();
  LOG4CXX_INFO(flog::rib, "Harq status");
  std::ostringstream oss;
  for (int i = 0; i < 8; i++) {
    oss << "  |  " << i;
  }
  LOG4CXX_INFO(flog::rib, oss.str() << "  |");
  oss.str(" ");
  oss.clear();
  for (int i = 0; i < 8; i++) {
    if (harq_stats_[0][i][0] == protocol::FLHS_ACK) {
      oss << " | " << "ACK";
    } else {
      oss << " | " << "NACK";
    }
  }
  LOG4CXX_INFO(flog::rib, oss.str() << "  |");
}

std::string flexran::rib::ue_mac_rib_info::dump_stats_to_string() const {

  std::string str;

  str += "Rnti: ";
  str += std::to_string(rnti_);
  str += "\n";
  mac_stats_report_mutex_.lock();
  str += mac_stats_report_.DebugString();
  mac_stats_report_mutex_.unlock();
  str += "\n";
  str += "Harq status";
  str += "\n";
  
  for (int i = 0; i < 8; i++) {
    str +=  "  |   ";
    str += std::to_string(i);
  }

  str += "   |   ";
  str += "\n";
  str += " ";
  for (int i = 0; i < 8; i++) {
    if (harq_stats_[0][i][0] == protocol::FLHS_ACK) {
      str += " | ";
      str += "ACK";
    } else {
      str += " | ";
      str += "NACK";
    }
  }
  str += "  |";
  str += "\n";

  return str;

}

std::string flexran::rib::ue_mac_rib_info::dump_stats_to_json_string() const
{
  std::string mac_stats;
  mac_stats_report_mutex_.lock();
  google::protobuf::util::MessageToJsonString(mac_stats_report_, &mac_stats, google::protobuf::util::JsonPrintOptions());
  mac_stats_report_mutex_.unlock();
  std::array<std::string, 8> harq;

  for (int i = 0; i < 8; i++) {
    harq[i] = harq_stats_[0][i][0] == protocol::FLHS_ACK ? "\"ACK\"" : "\"NACK\"";
  }

  return format_stats_to_json(rnti_, mac_stats, harq);
}

std::string flexran::rib::ue_mac_rib_info::format_stats_to_json(
    rnti_t rnti,
    const std::string& mac_stats,
    const std::array<std::string, 8>& harq)
{
  std::string str;
  str += "{\"rnti\": ";
  str += std::to_string(rnti);
  str += ",\"mac_stats\":";
  str += mac_stats;
  str += ",\"harq\":[";
  for (auto it = harq.begin(); it != harq.end(); it++) {
    if (it != harq.begin()) str += ",";
    str += *it;
  }
  str += "]}";
  return str;
}

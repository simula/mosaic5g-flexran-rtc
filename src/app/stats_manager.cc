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

/*! \file    stats_manager.cc
 *  \brief   request stats from new agents, helper for stats_manager_calls
 *  \authors Xenofon Foukas, Navid Nikaein, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           robert.schmidt@eurecom.fr
 */

#include <iostream>

#include "stats_manager.h"
#include "flexran.pb.h"
#include <google/protobuf/util/json_util.h>
namespace proto_util = google::protobuf::util;

#include "flexran_log.h"

flexran::app::stats::stats_manager::stats_manager(const flexran::rib::Rib& rib,
    const flexran::core::requests_manager& rm, flexran::event::subscription &s)
  : component(rib, rm, s)
{
  event_sub_.subscribe_bs_add(
      boost::bind(&flexran::app::stats::stats_manager::bs_add, this, _1));
  // Other possibility:
  //auto f = [this](uint64_t bs_id) { this->bs_add(bs_id); };
  //sub.subscribe_bs_add(boost::bind<void>(f, _1));
  event_sub_.subscribe_bs_remove(
      boost::bind(&flexran::app::stats::stats_manager::bs_remove, this, _1));
}

protocol::flex_complete_stats_request_repeated flexran::app::stats::stats_manager::default_stats_request()
{
  const uint32_t periodicity = 100;

  protocol::flex_complete_stats_request_repeated reqs;
  protocol::flex_complete_stats_request *req = reqs.add_reports();
  req->set_report_frequency(protocol::FLSRF_PERIODICAL);
  req->set_sf(periodicity);
  std::vector<protocol::flex_ue_stats_type> ue_reports{
      protocol::FLUST_PHR,              protocol::FLUST_DL_CQI,
      protocol::FLUST_BSR,              protocol::FLUST_RLC_BS,
      protocol::FLUST_MAC_CE_BS,        protocol::FLUST_UL_CQI,
      protocol::FLUST_RRC_MEASUREMENTS, protocol::FLUST_PDCP_STATS,
      protocol::FLUST_MAC_STATS,        protocol::FLUST_GTP_STATS,
      protocol::FLUST_S1AP_STATS};
  for (auto f : ue_reports)
    req->add_ue_reports(f);
  req->add_cell_reports(protocol::FLCST_NOISE_INTERFERENCE);
  return reqs;
}

void flexran::app::stats::stats_manager::bs_add(uint64_t bs_id)
{
  const uint32_t xid = 0;
  protocol::flex_complete_stats_request_repeated reqs = default_stats_request();
  for (const auto& r : reqs.reports())
    push_complete_stats_request(bs_id, xid, r);
  LOG4CXX_INFO(flog::app, "Sent periodical stats request to BS " << bs_id
      << " (xid " << xid << ")");
  bs_list_.insert({bs_id, reqs});
}

void flexran::app::stats::stats_manager::push_complete_stats_request(
    uint64_t bs_id, uint32_t xid, const protocol::flex_complete_stats_request& req)
{
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_STATS_REQUEST);
  header->set_version(0);
  // We need to store the xid for keeping context info
  header->set_xid(xid);

  protocol::flex_complete_stats_request *complete_stats_request(new protocol::flex_complete_stats_request);
  complete_stats_request->set_report_frequency(req.report_frequency());
  complete_stats_request->set_sf(req.sf());
  uint32_t ue_flags = 0;
  for (auto f : req.ue_reports())
    ue_flags |= f;
  complete_stats_request->set_ue_report_flags(ue_flags);
  uint32_t cell_flags = 0;
  for (auto f : req.cell_reports())
    cell_flags |= f;
  complete_stats_request->set_cell_report_flags(cell_flags);

  protocol::flex_stats_request *stats_request_msg(new protocol::flex_stats_request);
  stats_request_msg->set_allocated_header(header);
  stats_request_msg->set_type(protocol::FLST_COMPLETE_STATS);
  stats_request_msg->set_allocated_complete_stats_request(complete_stats_request);
  protocol::flexran_message msg;
  msg.set_msg_dir(protocol::INITIATING_MESSAGE);
  msg.set_allocated_stats_request_msg(stats_request_msg);
  req_manager_.send_message(bs_id, msg);
}

void flexran::app::stats::stats_manager::remove_complete_stats_request(
    uint64_t bs_id, uint32_t xid)
{
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_STATS_REQUEST);
  header->set_version(0);
  header->set_xid(xid);

  protocol::flex_complete_stats_request *off_msg(new protocol::flex_complete_stats_request);
  off_msg->set_report_frequency(protocol::FLSRF_OFF);

  protocol::flex_stats_request *stats_request_msg(new protocol::flex_stats_request);
  stats_request_msg->set_allocated_header(header);
  stats_request_msg->set_type(protocol::FLST_COMPLETE_STATS);
  stats_request_msg->set_allocated_complete_stats_request(off_msg);

  protocol::flexran_message out_message;
  out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message.set_allocated_stats_request_msg(stats_request_msg);
  req_manager_.send_message(bs_id, out_message);

}

void flexran::app::stats::stats_manager::bs_remove(uint64_t bs_id)
{
  auto it = bs_list_.find(bs_id);
  if (it == bs_list_.end()) return; /* not found */

  bs_list_.erase(it);
}

std::string flexran::app::stats::stats_manager::all_stats_to_string() const
{
  return all_enb_configs_to_string() + "\n\n\n" + all_mac_configs_to_string() + "\n";
}

std::string flexran::app::stats::stats_manager::all_stats_to_json_string() const
{
  return flexran::rib::Rib::format_statistics_to_json(
      std::chrono::system_clock::now(),
      rib_.dump_all_enb_configurations_to_json_string(),
      rib_.dump_all_mac_stats_to_json_string()
  );
}

bool flexran::app::stats::stats_manager::stats_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const
{
  std::string json1, json2;
  out = "{}";
  bool found = rib_.dump_enb_configurations_by_bs_id_to_json_string(bs_id, json1);
  if (!found) return false;
  found = rib_.dump_mac_stats_by_bs_id_to_json_string(bs_id, json2);
  if (!found) return false;
  out = flexran::rib::Rib::format_statistics_to_json(
      std::chrono::system_clock::now(), json1, json2
  );
  return true;
}

std::string flexran::app::stats::stats_manager::all_enb_configs_to_string() const
{
  std::string str;
  str += "**************************\n";
  str += "* BS/Cell Configurations *\n";
  str += "**************************\n";
  str += rib_.dump_all_enb_configurations_to_string();

  return str;
}

std::string flexran::app::stats::stats_manager::all_enb_configs_to_json_string() const
{
  return flexran::rib::Rib::format_statistics_to_json(
      std::chrono::system_clock::now(),
      rib_.dump_all_enb_configurations_to_json_string(), ""
  );
}

bool flexran::app::stats::stats_manager::enb_configs_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_enb_configurations_by_bs_id_to_json_string(bs_id, json);
  out = flexran::rib::Rib::format_statistics_to_json(
      std::chrono::system_clock::now(), json, "");
  return found;
}

std::string flexran::app::stats::stats_manager::all_mac_configs_to_string() const
{
  std::string str;
  str += "***************\n";
  str += "UE statistics\n";
  str += "****************\n";
  str += rib_.dump_all_mac_stats_to_string();

  return str;
}

std::string flexran::app::stats::stats_manager::all_mac_configs_to_json_string() const
{
  return flexran::rib::Rib::format_statistics_to_json(std::chrono::system_clock::now(),
      "", rib_.dump_all_mac_stats_to_json_string()
  );
}

bool flexran::app::stats::stats_manager::mac_configs_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_mac_stats_by_bs_id_to_json_string(bs_id, json);
  out = flexran::rib::Rib::format_statistics_to_json(std::chrono::system_clock::now(), "", json);
  return found;
}

bool flexran::app::stats::stats_manager::ue_stats_by_rnti_by_bs_id_to_json_string(flexran::rib::rnti_t rnti, std::string& out, uint64_t bs_id) const
{
  return rib_.dump_ue_by_rnti_by_bs_id_to_json_string(rnti, out, bs_id);
}

uint64_t flexran::app::stats::stats_manager::parse_bs_agent_id(const std::string& bs_agent_id_s) const
{
  return rib_.parse_enb_agent_id(bs_agent_id_s);
}

bool flexran::app::stats::stats_manager::parse_rnti_imsi(uint64_t bs_id, const std::string& rnti_imsi_s,
    flexran::rib::rnti_t& rnti) const
{
  return rib_.get_bs(bs_id)->parse_rnti_imsi(rnti_imsi_s, rnti);
}

bool flexran::app::stats::stats_manager::parse_rnti_imsi_find_bs(const std::string& rnti_imsi_s,
    flexran::rib::rnti_t& rnti, uint64_t& bs_id) const
{
  for (uint64_t xbs_id: rib_.get_available_base_stations()) {
    if (rib_.get_bs(xbs_id)->parse_rnti_imsi(rnti_imsi_s, rnti)) {
      bs_id = xbs_id;
      return true;
    }
  }
  return false;
}

bool flexran::app::stats::stats_manager::get_stats_requests(
    const std::string& bs, std::string& resp) const
{
  auto bsit = bs_list_.find(rib_.parse_enb_agent_id(bs));
  if (bsit == bs_list_.end()) {
    resp = "can not find BS";
    return false;
  }

  auto ret = proto_util::MessageToJsonString(bsit->second, &resp, proto_util::JsonPrintOptions());
  if (ret != proto_util::Status::OK) {
    resp = "ProtoBuf parser error";
    return false;
  }
  return true;
}

bool flexran::app::stats::stats_manager::set_stats_requests(
    const std::string& bs, const std::string& policy, std::string& error_reason)
{
  const uint64_t bs_id = rib_.parse_enb_agent_id(bs);
  if (bs_id == 0) {
    error_reason = "can not find BS";
    return false;
  }

  protocol::flex_complete_stats_request_repeated proto;
  auto ret = proto_util::JsonStringToMessage(policy, &proto, proto_util::JsonParseOptions());
  if (ret != proto_util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf  message:" << ret.ToString());
    return false;
  }

  /* remove all old requests */
  auto it = bs_list_.find(bs_id);
  if (it != bs_list_.end()) {
    for (int xid = 0; xid < it->second.reports().size(); ++xid) {
      remove_complete_stats_request(bs_id, xid);
      LOG4CXX_INFO(flog::app, "Remove stats request (xid " << xid << ") of BS " << bs_id);
    }
    it->second.mutable_reports()->Clear();
  } else {
    LOG4CXX_WARN(flog::app, "no previous stats_req configuration for BS " << bs_id);
  }

  /* send new requests and store */
  uint32_t xid = 0;
  for (const auto& r : proto.reports()) {
    push_complete_stats_request(bs_id, xid, r);
    LOG4CXX_INFO(flog::app, "Sent periodical stats request to BS " << bs_id
        << " (xid " << xid << ")");
    it->second.mutable_reports()->Add()->CopyFrom(r);
    xid++;
  }
  return true;
}

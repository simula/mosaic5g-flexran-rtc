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

#include "flexran_log.h"

void flexran::app::stats::stats_manager::periodic_task() {

  // Simply request stats for any registered eNB and print them
  std::set<int> current_agents = rib_.get_available_agents();

  for (auto agent_id : current_agents) {
    auto it = agent_list_.find(agent_id);
    if (it == agent_list_.end()) {
      agent_list_.insert(agent_id);
      // Make a new stats request for the newly added agents
      protocol::flex_header *header(new protocol::flex_header);
      header->set_type(protocol::FLPT_STATS_REQUEST);
      header->set_version(0);
      // We need to store the xid for keeping context info
      header->set_xid(0);
      
      protocol::flex_complete_stats_request *complete_stats_request(new protocol::flex_complete_stats_request);
      complete_stats_request->set_report_frequency(protocol::FLSRF_CONTINUOUS);
      complete_stats_request->set_sf(2);
      int ue_flags = 0;
      ue_flags |= protocol::FLUST_PHR;
      ue_flags |= protocol::FLUST_DL_CQI;
      ue_flags |= protocol::FLUST_BSR;
      ue_flags |= protocol::FLUST_RLC_BS;
      ue_flags |= protocol::FLUST_MAC_CE_BS;
      ue_flags |= protocol::FLUST_UL_CQI;
      ue_flags |= protocol::FLUST_RRC_MEASUREMENTS;
      ue_flags |= protocol::FLUST_PDCP_STATS;
      ue_flags |= protocol::FLUST_MAC_STATS;

      
      complete_stats_request->set_ue_report_flags(ue_flags);
      int cell_flags = 0;
      cell_flags |= protocol::FLCST_NOISE_INTERFERENCE;
      complete_stats_request->set_cell_report_flags(cell_flags);
      protocol::flex_stats_request *stats_request_msg(new protocol::flex_stats_request);
      stats_request_msg->set_allocated_header(header);
      stats_request_msg->set_type(protocol::FLST_COMPLETE_STATS);
      stats_request_msg->set_allocated_complete_stats_request(complete_stats_request);
      protocol::flexran_message msg;
      msg.set_msg_dir(protocol::INITIATING_MESSAGE);
      msg.set_allocated_stats_request_msg(stats_request_msg);
      req_manager_.send_message(agent_id, msg);
      LOG4CXX_INFO(flog::app, "Time to make a new request for stats");
    }
  }
}

std::string flexran::app::stats::stats_manager::all_stats_to_string() const
{
  return all_enb_configs_to_string() + "\n\n\n" + all_mac_configs_to_string() + "\n";
}

std::string flexran::app::stats::stats_manager::all_stats_to_json_string() const
{
  return "{" + rib_.dump_all_enb_configurations_to_json_string() + ","
    + rib_.dump_all_mac_stats_to_json_string() + "}";
}

bool flexran::app::stats::stats_manager::stats_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const
{
  std::string json1, json2;
  out = "{}";
  bool found = rib_.dump_enb_configurations_by_agent_id_to_json_string(agent_id, json1);
  if (!found) return false;
  found = rib_.dump_mac_stats_by_agent_id_to_json_string(agent_id, json2);
  if (!found) return false;
  out = "{" + json1 + "," + json2 + "}";
  return true;
}

std::string flexran::app::stats::stats_manager::all_enb_configs_to_string() const
{
  std::string str;
  str += "*************************\n";
  str += "Agent/Cell Configurations\n";
  str += "*************************\n";
  str += rib_.dump_all_enb_configurations_to_string();

  return str;
}

std::string flexran::app::stats::stats_manager::all_enb_configs_to_json_string() const
{
  return "{" + rib_.dump_all_enb_configurations_to_json_string() + "}";
}

bool flexran::app::stats::stats_manager::enb_configs_by_agent_id_to_json_string(int agent_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_enb_configurations_by_agent_id_to_json_string(agent_id, json);
  out = "{" + json + "}";
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
  return "{" + rib_.dump_all_mac_stats_to_json_string() + "}";
}

bool flexran::app::stats::stats_manager::mac_configs_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_mac_stats_by_agent_id_to_json_string(agent_id, json);
  out = "{" + json + "}";
  return found;
}

bool flexran::app::stats::stats_manager::ue_stats_by_rnti_by_agent_id_to_json_string(flexran::rib::rnti_t rnti, std::string& out, int agent_id) const
{
  return rib_.dump_ue_by_rnti_by_agent_id_to_json_string(rnti, out, agent_id);
}

int flexran::app::stats::stats_manager::parse_enb_agent_id(const std::string& enb_agent_id_s) const
{
  return rib_.parse_enb_agent_id(enb_agent_id_s);
}

bool flexran::app::stats::stats_manager::parse_rnti_imsi(int agent_id, const std::string& rnti_imsi_s,
    flexran::rib::rnti_t& rnti) const
{
  return rib_.get_agent(agent_id)->parse_rnti_imsi(rnti_imsi_s, rnti);
}

bool flexran::app::stats::stats_manager::parse_rnti_imsi_find_agent(const std::string& rnti_imsi_s,
    flexran::rib::rnti_t& rnti, int& agent_id) const
{
  std::set<int> current_agents = rib_.get_available_agents();
  for (int agent: current_agents) {
    if (rib_.get_agent(agent)->parse_rnti_imsi(rnti_imsi_s, rnti)) {
      agent_id = agent;
      return true;
    }
  }
  return false;
}

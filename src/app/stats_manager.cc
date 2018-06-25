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

bool flexran::app::stats::stats_manager::stats_by_enb_id_to_json_string(uint64_t enb_id, std::string& out) const
{
  std::string json1, json2;
  out = "{}";
  bool found = rib_.dump_enb_configurations_by_enb_id_to_json_string(enb_id, json1);
  if (!found) return false;
  found = rib_.dump_mac_stats_by_enb_id_to_json_string(enb_id, json2);
  if (!found) return false;
  out = "{" + json1 + "," + json2 + "}";
  return true;
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

bool flexran::app::stats::stats_manager::enb_configs_by_enb_id_to_json_string(uint64_t enb_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_enb_configurations_by_enb_id_to_json_string(enb_id, json);
  out = "{" + json + "}";
  return found;
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

bool flexran::app::stats::stats_manager::mac_configs_by_enb_id_to_json_string(uint64_t enb_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_mac_stats_by_enb_id_to_json_string(enb_id, json);
  out = "{" + json + "}";
  return found;
}

bool flexran::app::stats::stats_manager::mac_configs_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const
{
  std::string json;
  bool found = rib_.dump_mac_stats_by_agent_id_to_json_string(agent_id, json);
  out = "{" + json + "}";
  return found;
}

bool flexran::app::stats::stats_manager::ue_stats_by_rnti_to_json_string(flexran::rib::rnti_t rnti, std::string& out) const
{
  return rib_.dump_ue_by_rnti_to_json_string(rnti, out);
}

bool flexran::app::stats::stats_manager::ue_stats_by_rnti_by_enb_id_to_json_string(flexran::rib::rnti_t rnti, std::string& out, uint64_t enb_id) const
{
  return rib_.dump_ue_by_rnti_by_enb_id_to_json_string(rnti, out, enb_id);
}

bool flexran::app::stats::stats_manager::ue_stats_by_rnti_by_agent_id_to_json_string(flexran::rib::rnti_t rnti, std::string& out, int agent_id) const
{
  return rib_.dump_ue_by_rnti_by_agent_id_to_json_string(rnti, out, agent_id);
}

bool flexran::app::stats::stats_manager::ue_stats_by_imsi_to_json_string(uint64_t imsi, std::string& out) const
{
  return rib_.dump_ue_by_imsi_to_json_string(imsi, out);
}

bool flexran::app::stats::stats_manager::ue_stats_by_imsi_by_enb_id_to_json_string(uint64_t imsi, std::string& out, uint64_t enb_id) const
{
  return rib_.dump_ue_by_imsi_by_enb_id_to_json_string(imsi, out, enb_id);
}

bool flexran::app::stats::stats_manager::ue_stats_by_imsi_by_agent_id_to_json_string(uint64_t imsi, std::string& out, int agent_id) const
{
  return rib_.dump_ue_by_imsi_by_agent_id_to_json_string(imsi, out, agent_id);
}

int flexran::app::stats::stats_manager::parse_enb_agent_id(const std::string& enb_agent_id_s) const
{
  return rib_.parse_enb_agent_id(enb_agent_id_s);
}

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

/*! \file    rib.cc
 *  \brief   Ran Information Base: the controller's view on all agents
 *  \authors Xenofon Foukas, Navid Nikaein, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           robert.schmidt@eurecom.fr
 */

#include "rib.h"
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <sstream>

bool flexran::rib::Rib::add_pending_agent(std::shared_ptr<agent_info> ai)
{
  if (ai->bs_id == 0)
    return false;
  if (agent_is_pending(ai->agent_id))
    return false;
  if (ai->capabilities.size() < 1)
    return false;
  for (auto p: pending_agents_) {
    /* check that there is no existing pending agent with the same ID and
     * overlapping capabilities */
    if (p->bs_id == ai->bs_id
        && !p->capabilities.orthogonal(ai->capabilities)) {
      return false;
    }
  }

  pending_agents_.emplace(ai);
  return true;
}

bool flexran::rib::Rib::agent_is_pending(int agent_id) const
{
  auto search = std::find_if(pending_agents_.begin(), pending_agents_.end(),
      [agent_id] (std::shared_ptr<agent_info> ai)
      { return agent_id == ai->agent_id; }
    );
  return search != pending_agents_.end();
}

void flexran::rib::Rib::remove_pending_agent(int agent_id)
{
  auto search = std::find_if(pending_agents_.begin(), pending_agents_.end(),
      [agent_id] (std::shared_ptr<agent_info> ai)
      { return agent_id == ai->agent_id; }
    );
  if (search == pending_agents_.end()) return;
  pending_agents_.erase(search);
}

void flexran::rib::Rib::remove_pending_agents(uint64_t bs_id)
{
  std::set<std::shared_ptr<agent_info>>::const_iterator search;
  while ((search = std::find_if(pending_agents_.begin(), pending_agents_.end(),
                       [bs_id] (std::shared_ptr<agent_info> ai)
                       { return bs_id == ai->bs_id; }
                   )) != pending_agents_.end()) {
    pending_agents_.erase(search);
  }
}

bool flexran::rib::Rib::new_eNB_config_entry(uint64_t bs_id)
{
  /* ignore if such a BS already exists */
  if (get_bs(bs_id) != nullptr) return false;

  std::set<std::shared_ptr<agent_info>> agents;
  /* get all agents matching bs_id */
  std::for_each(pending_agents_.begin(), pending_agents_.end(),
      [bs_id,&agents] (std::shared_ptr<agent_info> ai)
      { if (bs_id == ai->bs_id) agents.insert(ai); }
    );

  if (agents.size() == 1 && (*agents.begin())->capabilities.is_complete()) {
    /* create new bs with this agent, remove pending agent, create entry for
     * known agents */
    eNB_configs_.emplace(
        (*agents.begin())->bs_id,
        std::make_shared<enb_rib_info>((*agents.begin())->bs_id, agents)
    );
    pending_agents_.erase(*agents.begin());
    agent_configs_.emplace((*agents.begin())->agent_id, *agents.begin());
    return true;
  }

  if (agents.size() > 1) {
    agent_capabilities caps((*agents.begin())->capabilities);
    for (auto it = agents.begin(); it != agents.end(); ++it) {
      if (it == agents.begin()) continue;
      /* test that capabilities don't overlap, if yes, add */
      if (caps.orthogonal((*it)->capabilities)) caps.merge_in((*it)->capabilities);
      else throw std::runtime_error("overlapping capabilities detected");
    }
    if (caps.is_complete()) {
      eNB_configs_.emplace(std::make_pair(
          (*agents.begin())->bs_id,
          std::make_shared<enb_rib_info>((*agents.begin())->bs_id, agents)
        )
      );
      for (auto a : agents) {
        pending_agents_.erase(a);
        agent_configs_.emplace(a->agent_id, a);
      }
      return true;
    }
  }

  return false;
}

bool flexran::rib::Rib::has_eNB_config_entry(uint64_t bs_id) const
{
  return get_bs(bs_id) != nullptr;
}

bool flexran::rib::Rib::remove_eNB_config_entry(int agent_id)
{
  const auto it = agent_configs_.find(agent_id);
  if (it == agent_configs_.end()) return false;
  const std::shared_ptr<agent_info> disconnected = it->second;

  /* get all agents for this BS, remove corresponding enb_rib_info and
   * agent_configs_ and put agents that are still connected into pending */
  std::set<std::shared_ptr<agent_info>> all = eNB_configs_.find(disconnected->bs_id)->second->get_agents();
  eNB_configs_.erase(disconnected->bs_id);
  for (auto b: all)
    agent_configs_.erase(b->agent_id);
  all.erase(disconnected);
  for (auto b: all)
    pending_agents_.insert(b);
  return true;
}

std::set<uint64_t> flexran::rib::Rib::get_available_base_stations() const
{
  std::set<uint64_t> agents;
  for (auto it : eNB_configs_) {
    agents.insert(it.first);
  }
  return agents;
}

std::shared_ptr<flexran::rib::enb_rib_info>
flexran::rib::Rib::get_bs(uint64_t bs_id) const
{
  auto it = eNB_configs_.find(bs_id);
  if (it == eNB_configs_.end()) return nullptr;

  return it->second;
}

std::shared_ptr<flexran::rib::enb_rib_info>
flexran::rib::Rib::get_bs_from_agent(int agent_id) const
{
  uint64_t bs_id = get_bs_id(agent_id);
  if (bs_id == 0) return nullptr;
  return get_bs(bs_id);
}

std::shared_ptr<flexran::rib::agent_info>
flexran::rib::Rib::get_agent(int agent_id) const
{
  auto it = agent_configs_.find(agent_id);
  if (it == agent_configs_.end()) return nullptr;

  return it->second;
}

void flexran::rib::Rib::dump_mac_stats() const {
  for (auto enb_config : eNB_configs_) {
    enb_config.second->dump_mac_stats();
  }
}

std::string flexran::rib::Rib::dump_all_mac_stats_to_string() const {

  std::string str;
  
  for (auto enb_config : eNB_configs_) {
    str += enb_config.second->dump_mac_stats_to_string();
    str += "\n";
  }

  return str;
}

std::string flexran::rib::Rib::dump_all_mac_stats_to_json_string() const
{
  std::vector<std::string> mac_stats;
  mac_stats.reserve(eNB_configs_.size());
  std::transform(eNB_configs_.begin(), eNB_configs_.end(), std::back_inserter(mac_stats),
      [] (const std::pair<int, std::shared_ptr<enb_rib_info>>& enb_config)
      { return enb_config.second->dump_mac_stats_to_json_string(); }
  );

  return format_mac_stats_to_json(mac_stats);
}

bool flexran::rib::Rib::dump_mac_stats_by_bs_id_to_json_string(uint64_t bs_id,
    std::string& out) const
{
  auto it = eNB_configs_.find(bs_id);
  if (it == eNB_configs_.end()) return false;

  out = format_mac_stats_to_json(std::vector<std::string>{it->second->dump_mac_stats_to_json_string()});
  return true;
}

std::string flexran::rib::Rib::format_mac_stats_to_json(
    const std::vector<std::string>& mac_stats_json)
{
  std::string str;
  str += "[";
  for (auto it = mac_stats_json.begin(); it != mac_stats_json.end(); it++) {
    if (it != mac_stats_json.begin()) str += ",";
    str += "{";
    str += *it;
    str += "}";
  }
  str += "]";
  return str;
}

void flexran::rib::Rib::dump_enb_configurations() const {
  for (auto eNB_config : eNB_configs_) {
    eNB_config.second->dump_configs();
  }
}

std::string flexran::rib::Rib::dump_all_enb_configurations_to_string() const {
  std::string str;

  for (auto eNB_config : eNB_configs_) {
    str += eNB_config.second->dump_configs_to_string();
    str += "\n";
  }

  return str;
}

std::string flexran::rib::Rib::dump_all_enb_configurations_to_json_string() const
{
  std::vector<std::string> enb_configurations;
  enb_configurations.reserve(eNB_configs_.size());
  std::transform(eNB_configs_.begin(), eNB_configs_.end(), std::back_inserter(enb_configurations),
      [] (const std::pair<int, std::shared_ptr<enb_rib_info>>& enb_config)
      { return enb_config.second->dump_configs_to_json_string(); }
  );

  return format_enb_configurations_to_json(enb_configurations);
}

bool flexran::rib::Rib::dump_enb_configurations_by_bs_id_to_json_string(
    uint64_t bs_id, std::string& out) const
{
  auto it = eNB_configs_.find(bs_id);
  if (it == eNB_configs_.end()) return false;

  out = format_enb_configurations_to_json(std::vector<std::string>{it->second->dump_configs_to_json_string()});
  return true;
}

std::string flexran::rib::Rib::format_enb_configurations_to_json(
    const std::vector<std::string>& enb_configurations_json)
{
  std::string str;
  str += "[";
  for (auto it = enb_configurations_json.begin(); it != enb_configurations_json.end(); it++) {
    if (it != enb_configurations_json.begin()) str += ",";
    str += "{";
    str += *it;
    str += "}";
  }
  str += "]";
  return str;
}

std::string flexran::rib::Rib::format_statistics_to_json(
    std::chrono::time_point<std::chrono::system_clock> t,
    const std::string& configurations,
    const std::string& mac_stats)
{
  std::string str = "{";
  str += "\"date_time\":\"" + format_date_time(t) + "\",";
  if (!configurations.empty())
    str += "\"eNB_config\":" + configurations;
  if (!configurations.empty() && !mac_stats.empty())
    str += ",";
  if (!mac_stats.empty())
    str += "\"mac_stats\":" + mac_stats;
  str += "}";
  return str;
}

std::string flexran::rib::Rib::format_date_time(std::chrono::time_point<std::chrono::system_clock> t)
{
  std::ostringstream oss;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()) % 1000;
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  oss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S.")
    << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

bool flexran::rib::Rib::dump_ue_by_rnti_by_bs_id_to_json_string(
    rnti_t rnti, std::string& out, uint64_t bs_id) const
{
  auto it = eNB_configs_.find(bs_id);
  if (it == eNB_configs_.end()) return false;
  return it->second->dump_ue_spec_stats_by_rnti_to_json_string(rnti, out);
}

uint64_t flexran::rib::Rib::get_bs_id(int agent_id) const
{
  auto it = agent_configs_.find(agent_id);
  if (it == agent_configs_.end()) return 0;

  return it->second->bs_id;
}

uint64_t flexran::rib::Rib::parse_enb_agent_id(const std::string& enb_agent_id_s) const
{
  /* -> return last eNB_config entry */
  if (enb_agent_id_s == "-1") {
    return eNB_configs_.empty() ? 0 : std::prev(eNB_configs_.end())->first;
  }

  uint64_t enb_id;
  try {
    if (enb_agent_id_s.substr(0, 2) == "0x")
      enb_id = std::stoll(enb_agent_id_s, 0, 16);
    else
      enb_id = std::stoll(enb_agent_id_s);
  } catch (const std::invalid_argument& e) {
    return 0;
  }

  /* shorter than length limit -> assume it is agent ID */
  if (enb_agent_id_s.length() < AGENT_ID_LENGTH_LIMIT)
    return get_bs_id(enb_id);

  if (!get_bs(enb_id)) return 0;
  return enb_id;
}

uint64_t flexran::rib::Rib::parse_bs_id(const std::string& bs_id_s) const
{
  /* -> return last eNB_config entry */
  if (bs_id_s == "-1") {
    return eNB_configs_.empty() ? 0 : std::prev(eNB_configs_.end())->first;
  }
  uint64_t enb_id;
  try {
    if (bs_id_s.substr(0, 2) == "0x")
      enb_id = std::stoll(bs_id_s, 0, 16);
    else
      enb_id = std::stoll(bs_id_s);
  } catch (const std::invalid_argument& e) {
    return 0;
  }
  if (!get_bs(enb_id)) return 0;
  return enb_id;
}

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

/*! \file    rib.h
 *  \brief   Ran Information Base: the controller's view on all agents
 *  \authors Xenofon Foukas, Navid Nikaein, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           robert.schmidt@eurecom.fr
 */

#ifndef RIB_H_
#define RIB_H_

#include <map>
#include <set>

#include "enb_rib_info.h"
#include "agent_info.h"
#include <memory>
#include <set>
#include <chrono>

namespace flexran {

  namespace rib {

    class Rib {
    public:

      // Pending agent methods
      bool add_pending_agent(std::shared_ptr<agent_info> ai);
      bool agent_is_pending(int agent_id) const;
      void remove_pending_agent(int agent_id);
      void remove_pending_agents(uint64_t bs_id);
      std::size_t get_num_pending_agents() const { return pending_agents_.size(); }
      
      // eNB config management
      bool new_eNB_config_entry(uint64_t bs_id);
      bool has_eNB_config_entry(uint64_t bs_id) const;
      bool remove_eNB_config_entry(int agent_id);
      
      std::set<uint64_t> get_available_base_stations() const;
      std::map<int, std::shared_ptr<agent_info>> get_agents() const { return agent_configs_; }
      
      std::shared_ptr<enb_rib_info> get_bs(uint64_t bs_id) const;
      std::shared_ptr<enb_rib_info> get_bs_from_agent(int agent_id) const;
      std::shared_ptr<agent_info>   get_agent(int agent_id) const;
      
      void dump_mac_stats() const;
      
      void dump_enb_configurations() const;

      std::string dump_all_mac_stats_to_string() const;
      std::string dump_all_mac_stats_to_json_string() const;
      bool dump_mac_stats_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const;

      static std::string format_mac_stats_to_json(const std::vector<std::string>& mac_stats_json);
      
      std::string dump_all_enb_configurations_to_string() const;
      std::string dump_all_enb_configurations_to_json_string() const;
      bool dump_enb_configurations_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const;

      static std::string format_enb_configurations_to_json(const std::vector<std::string>& enb_configurations_json);

      bool dump_ue_by_rnti_by_bs_id_to_json_string(rnti_t rnti, std::string& out, uint64_t bs_id) const;

      uint64_t get_bs_id(int agent_id) const;
      uint64_t parse_enb_agent_id(const std::string& enb_agent_id_s) const;
      uint64_t parse_bs_id(const std::string& bs_id_s) const;
      static std::string format_statistics_to_json(
          std::chrono::time_point<std::chrono::system_clock> t,
          const std::string& configurations, const std::string& mac_stats);
      static std::string format_date_time(std::chrono::time_point<std::chrono::system_clock> t);
      
    private:
      std::map<uint64_t, std::shared_ptr<enb_rib_info>> eNB_configs_;
      std::map<int, std::shared_ptr<agent_info>> agent_configs_;
      std::set<std::shared_ptr<agent_info>> pending_agents_;

      static constexpr const size_t AGENT_ID_LENGTH_LIMIT = 4;
      
    };

  }
  
}

#endif

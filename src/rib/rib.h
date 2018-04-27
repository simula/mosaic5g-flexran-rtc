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
#include <memory>
#include <set>

namespace flexran {

  namespace rib {

    class Rib {
    public:

      // Pending agent methods
      void add_pending_agent(int agent_id);
      void remove_pending_agent(int agent_id);
      bool agent_is_pending(int agent_id);
      
      // eNB config management
      void new_eNB_config_entry(int agent_id);
      bool has_eNB_config_entry(int agent_id) const;
      void remove_eNB_config_entry(int agent_id);
      void eNB_config_update(int agent_id,
			     const protocol::flex_enb_config_reply& enb_config_update);
      void ue_config_update(int agent_id,
			    const protocol::flex_ue_config_reply& ue_config_update);
      void ue_config_update(int agent_id,
			    const protocol::flex_ue_state_change& ue_state_change);
      void lc_config_update(int agent_id,
			    const protocol::flex_lc_config_reply& lc_config_update);
      
      void update_liveness(int agent_id);
      
      void set_subframe_updates(int agent_id,
				const protocol::flex_sf_trigger& sf_trigger_msg);
      
      void mac_stats_update(int agent_id,
			    const protocol::flex_stats_reply& mac_stats_update);
      
      std::set<int> get_available_agents() const;
      
      std::shared_ptr<enb_rib_info> get_agent(int agent_id) const;
      
      void dump_mac_stats() const;
      
      void dump_enb_configurations() const;

      std::string dump_all_mac_stats_to_string() const;
      std::string dump_all_mac_stats_to_json_string() const;
      bool dump_mac_stats_by_agent_id_to_json_string(int agent_id, std::string& out) const;

      static std::string format_mac_stats_to_json(const std::vector<std::string>& mac_stats_json);
      
      std::string dump_all_enb_configurations_to_string() const;
      std::string dump_all_enb_configurations_to_json_string() const;
      bool dump_enb_configurations_by_agent_id_to_json_string(int agent_id, std::string& out) const;

      static std::string format_enb_configurations_to_json(const std::vector<std::string>& enb_configurations_json);

      bool dump_ue_by_rnti_by_agent_id_to_json_string(rnti_t rnti, std::string& out, int agent_id) const;

      int get_agent_id(uint64_t enb_id) const;
      int parse_enb_agent_id(const std::string& enb_agent_id_s) const;
      
    private:
      std::map<int, std::shared_ptr<enb_rib_info>>::const_iterator find_agent(uint64_t enb_id) const;
      
      std::map<int, std::shared_ptr<enb_rib_info>> eNB_configs_;
      std::set<int> pending_agents_;
      static constexpr const size_t AGENT_ID_LENGTH_LIMIT = 4;
      
    };

  }
  
}

#endif

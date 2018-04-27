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

/*! \file    stats_manager.h
 *  \brief   request stats from new agents, helper for stats_manager_calls
 *  \authors Xenofon Foukas, Navid Nikaein, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           robert.schmidt@eurecom.fr
 */

#ifndef STATS_MANAGER_H_
#define STATS_MANAGER_H_

#include <set>

#include "periodic_component.h"
#include "rib_common.h"

namespace flexran {

  namespace app {

    namespace stats {

      class stats_manager : public periodic_component {
	
      public:
	
      stats_manager(flexran::rib::Rib& rib, const flexran::core::requests_manager& rm)
      : periodic_component(rib, rm) {}
	
      void periodic_task();

      std::string all_stats_to_string() const;
      std::string all_stats_to_json_string() const;
      bool stats_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const;

      std::string all_enb_configs_to_string() const;
      std::string all_enb_configs_to_json_string() const;
      bool enb_configs_by_agent_id_to_json_string(int agent_id, std::string& out) const;

      std::string all_mac_configs_to_string() const;
      std::string all_mac_configs_to_json_string() const;
      bool mac_configs_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const;

      bool ue_stats_by_rnti_by_agent_id_to_json_string(flexran::rib::rnti_t rnti, std::string& out,
          int agent_id) const;

      // returns the agent_id of matching agent/enb ID string or -1 if not
      // found
      int parse_enb_agent_id(const std::string& enb_agent_id_s) const;
      /// returns true if the given RNTI/IMSI string for agent agent_id is
      /// found (saving the rnti), or false
      bool parse_rnti_imsi(int agent_id, const std::string& rnti_imsi_s, flexran::rib::rnti_t& rnti) const;
      /// returns true if the given RNTI/IMSI string in any agent agent_id is
      /// found (saving the rnti and agent_id), or false
      bool parse_rnti_imsi_find_agent(const std::string& rnti_imsi_s, flexran::rib::rnti_t& rnti,
          int& agent_id) const;

      private:

        std::set<int> agent_list_;

      };

    }

  }

}

#endif

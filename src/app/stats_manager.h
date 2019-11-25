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

#include "component.h"
#include "rib_common.h"
#include "subscription.h"

namespace protocol {
  class flex_complete_stats_request;
}

namespace flexran {

  namespace app {

    namespace stats {

      class stats_manager : public component {
	
      public:
	
      stats_manager(const rib::Rib& rib, const core::requests_manager& rm,
          event::subscription& sub);

      void bs_add(uint64_t bs_id);
      void bs_remove(uint64_t bs_id);

      std::string all_stats_to_string() const;
      std::string all_stats_to_json_string() const;
      bool stats_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const;

      std::string all_enb_configs_to_string() const;
      std::string all_enb_configs_to_json_string() const;
      bool enb_configs_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const;

      std::string all_mac_configs_to_string() const;
      std::string all_mac_configs_to_json_string() const;
      bool mac_configs_by_bs_id_to_json_string(uint64_t bs_id, std::string& out) const;

      bool ue_stats_by_rnti_by_bs_id_to_json_string(flexran::rib::rnti_t rnti, std::string& out,
          uint64_t bs_id) const;

      // returns the bs_id of matching agent/enb ID string or zero if not
      // found
      uint64_t parse_bs_agent_id(const std::string& bs_agent_id_s) const;
      /// returns true if the given RNTI/IMSI string for agent bs_id is
      /// found (saving the rnti), or false
      bool parse_rnti_imsi(uint64_t bs_id, const std::string& rnti_imsi_s, flexran::rib::rnti_t& rnti) const;
      /// returns true if the given RNTI/IMSI string in any BS bs_id is
      /// found (saving the rnti and bs_id), or false
      bool parse_rnti_imsi_find_bs(const std::string& rnti_imsi_s, flexran::rib::rnti_t& rnti,
          uint64_t& bs_id) const;

      bool get_stats_requests(const std::string& bs, std::string& resp) const;
      bool set_stats_requests(const std::string& bs, const std::string& policy,
          std::string& error_reason);

      private:
        void push_complete_stats_request(uint64_t bs_id, uint32_t xid,
            const protocol::flex_complete_stats_request& req);
        void remove_complete_stats_request(uint64_t bs_id, uint32_t xid);
        protocol::flex_complete_stats_request_repeated default_stats_request();

        std::map<uint64_t, protocol::flex_complete_stats_request_repeated> bs_list_;

      };

    }

  }

}

#endif

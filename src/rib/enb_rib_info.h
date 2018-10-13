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

/*! \file    enb_rib_info.h
 *  \brief   wrapper class for a cell's configuration
 *  \authors Xenofon Foukas, Navid Nikaein, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           robert.schmidt@eurecom.fr
 */

#ifndef ENB_RIB_INFO_H_
#define ENB_RIB_INFO_H_

#include <map>
#include <memory>
#include <mutex>
#include <chrono>
using st_clock = std::chrono::steady_clock;

#include "flexran.pb.h"
#include "rib_common.h"
#include "ue_mac_rib_info.h"
#include "cell_mac_rib_info.h"
#include "agent_info.h"

namespace flexran {

  namespace rib {

    class enb_rib_info {
    public:
      enb_rib_info(uint64_t bs_id, const std::set<std::shared_ptr<agent_info>>& agents);
      
      void update_eNB_config(const protocol::flex_enb_config_reply& enb_config_update);
      
      void update_UE_config(const protocol::flex_ue_config_reply& ue_config_update);

      void update_UE_config(const protocol::flex_ue_state_change& ue_state_change);
      
      void update_LC_config(const protocol::flex_lc_config_reply& lc_config_update);

      void update_liveness();

      void update_subframe(const protocol::flex_sf_trigger& sf_trigger);

      void update_mac_stats(const protocol::flex_stats_reply& mac_stats);
  
      bool need_to_query();

      void dump_mac_stats() const;

      std::string dump_mac_stats_to_string() const;

      std::string dump_mac_stats_to_json_string() const;

      static std::string format_mac_stats_to_json(uint64_t bs_id,
          const std::vector<std::string>& ue_mac_stats_json);

      void dump_configs() const;

      std::string dump_configs_to_string() const;

      std::string dump_configs_to_json_string() const;

      static std::string format_configs_to_json(uint64_t bs_id,
                                                const std::string& agent_info_json,
                                                const std::string& eNB_config_json,
                                                const std::string& ue_config_json,
                                                const std::string& lc_config_json);

      bool dump_ue_spec_stats_by_rnti_to_json_string(rnti_t rnti, std::string& out) const;

      frame_t get_current_frame() const { return current_frame_; }

      subframe_t get_current_subframe() const { return current_subframe_; }

      //! Access is only safe when the RIB is not active, i.e. within apps
      const protocol::flex_enb_config_reply& get_enb_config() const {return eNB_config_;}

      //! Access is only safe when the RIB is not active, i.e. within apps
      const protocol::flex_ue_config_reply& get_ue_configs() const {return ue_config_;}

      //! Access is only safe when the RIB is not active, i.e. within apps
      const protocol::flex_lc_config_reply& get_lc_configs() const {return lc_config_;}

      std::chrono::steady_clock::time_point last_active() const { return last_checked; }

      std::shared_ptr<ue_mac_rib_info> get_ue_mac_info(rnti_t rnti) const;

      cell_mac_rib_info& get_cell_mac_rib_info(uint16_t cell_id) {
	return cell_mac_info_[cell_id];
      }

      bool parse_rnti_imsi(const std::string& rnti_imsi_s, rnti_t& rnti) const;
      bool get_rnti(uint64_t imsi, rnti_t& rnti) const;
      bool has_dl_slice(uint32_t slice_id, uint16_t cell_id = 0) const;
      uint32_t num_dl_slices(uint16_t cell_id = 0) const;
      bool has_ul_slice(uint32_t slice_id, uint16_t cell_id = 0) const;
      uint32_t num_ul_slices(uint16_t cell_id = 0) const;

      std::set<std::shared_ptr<agent_info>> get_agents() const { return agents_; }
      uint64_t get_id() const { return bs_id_; }

    private:

      void clear_repeated_if_present(google::protobuf::Message *dst,
          const google::protobuf::Message& src);
      
    private:
      uint64_t bs_id_;
      std::set<std::shared_ptr<agent_info>> agents_;

      st_clock::time_point last_checked;
      /* Was: 500 for clock_t, have CLOCK_PER_SECOND == 1000000.
       * so: 500/1000000 = 0.5ms */
      const st_clock::duration time_to_query = std::chrono::microseconds(500);

      frame_t current_frame_;
      subframe_t current_subframe_;
      
      // eNB config structure
      protocol::flex_enb_config_reply eNB_config_;
      mutable std::mutex eNB_config_mutex_;
      // UE config structure
      protocol::flex_ue_config_reply ue_config_;
      mutable std::mutex ue_config_mutex_;
      // LC config structure
      protocol::flex_lc_config_reply lc_config_;
      mutable std::mutex lc_config_mutex_;
      
      std::map<rnti_t, std::shared_ptr<ue_mac_rib_info>> ue_mac_info_;

      cell_mac_rib_info cell_mac_info_[MAX_NUM_CC];
      static constexpr const size_t RNTI_ID_LENGTH_LIMIT = 6;
    };

  }

}
    
#endif

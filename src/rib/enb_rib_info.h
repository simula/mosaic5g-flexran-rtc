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

namespace flexran {

  namespace rib {

    class enb_rib_info {
    public:
      enb_rib_info(int agent_id);
      
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

      static std::string format_mac_stats_to_json(int agent_id, uint64_t enb_id,
          const std::vector<std::string>& ue_mac_stats_json);

      void dump_configs() const;

      std::string dump_configs_to_string() const;

      std::string dump_configs_to_json_string() const;

      static std::string format_configs_to_json(const std::string& eNB_config_json,
                                                const std::string& ue_config_json,
                                                const std::string& lc_config_json);

      bool dump_ue_spec_stats_by_rnti_to_json_string(rnti_t rnti, std::string& out) const;
      bool dump_ue_spec_stats_by_imsi_to_json_string(uint64_t imsi, std::string& out) const;

      frame_t get_current_frame() const { return current_frame_; }

      subframe_t get_current_subframe() const { return current_subframe_; }

      //! Access is only safe when the RIB is not active, i.e. within apps
      protocol::flex_enb_config_reply& get_enb_config() {return eNB_config_;}

      //! Access is only safe when the RIB is not active, i.e. within apps
      protocol::flex_ue_config_reply& get_ue_configs() {return ue_config_;}

      //! Access is only safe when the RIB is not active, i.e. within apps
      protocol::flex_lc_config_reply& get_lc_configs() {return lc_config_;}

      std::chrono::steady_clock::time_point last_active() const { return last_checked; }

      std::shared_ptr<ue_mac_rib_info> get_ue_mac_info(rnti_t rnti);

      cell_mac_rib_info& get_cell_mac_rib_info(uint16_t cell_id) {
	return cell_mac_info_[cell_id];
      }
      
    private:
      int agent_id_;

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
  
    };

  }

}
    
#endif

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

/*! \file    flexible_scheduler.h
 *  \brief   app for central scheduling and RRM calls helper
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef FLEXIBLE_SCHEDULER_H_
#define FLEXIBLE_SCHEDULER_H_

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"
#include "rib_common.h"

#include <atomic>

namespace flexran {

  namespace app {

    namespace scheduler {

      class flexible_scheduler : public periodic_component {

      public:

	flexible_scheduler(rib::Rib& rib, const core::requests_manager& rm)
	  : periodic_component(rib, rm), code_pushed_(false) {

	  central_scheduling.store(false);
	  
	}

	void periodic_task();

	//void reconfigure_agent_file(uint64_t bs_id, std::string policy_name);

	//void reconfigure_agent_string(uint64_t bs_id, std::string policy);

	//void enable_central_scheduling(bool central_sch);

        bool apply_slice_config_policy(uint64_t bs_id, const std::string& policy,
            std::string& error_reason);
        bool remove_slice(uint64_t bs_id, const std::string& policy,
            std::string& error_reason);
        bool change_ue_slice_association(uint64_t bs_id, const std::string& policy,
            std::string& error_reason);
        bool apply_cell_config_policy(uint64_t bs_id, const std::string& policy,
            std::string& error_reason);

	static int32_t tpc_accumulated;

        uint64_t parse_enb_agent_id(const std::string& enb_agent_id_s) const;
        uint64_t get_last_bs() const;
        bool parse_rnti_imsi(uint64_t bs_id, const std::string& rnti_imsi_s,
            flexran::rib::rnti_t& rnti) const;

      private:

	void run_central_scheduler();
	void push_code(uint64_t bs_id, std::string function_name, std::string lib_name);

        void push_cell_config_reconfiguration(uint64_t bs_id,
            const protocol::flex_cell_config& cell_config);
        void push_ue_config_reconfiguration(uint64_t bs_id,
            const protocol::flex_ue_config_reply& ue_config);
        static bool verify_dl_slice_config(const protocol::flex_dl_slice& s,
            std::string& error_message);
        static bool verify_dl_slice_removal(const protocol::flex_dl_slice& s,
            std::string& error_message);
        static bool verify_ul_slice_config(const protocol::flex_ul_slice& s,
            std::string& error_message);
        static bool verify_ul_slice_removal(const protocol::flex_ul_slice& s,
            std::string& error_message);
        bool verify_global_slice_percentage(uint64_t bs_id,
            const protocol::flex_slice_config& c, std::string& error_message);
        bool verify_global_dl_slice_percentage(
            const protocol::flex_slice_config& existing,
            const protocol::flex_slice_config& update,
            std::string& error_message);
        bool verify_global_ul_slice_percentage(
            const protocol::flex_slice_config& existing,
            const protocol::flex_slice_config& update,
            std::string& error_message);
        static bool verify_ue_slice_assoc_msg(const protocol::flex_ue_config& c,
            std::string& error_message);
        bool verify_rnti_imsi(uint64_t bs_id, protocol::flex_ue_config *c,
            std::string& error_message);
        bool try_add_first_rb(uint64_t bs_id, protocol::flex_ul_slice& slice);
        static bool verify_cell_config_for_restart(const protocol::flex_cell_config& c,
            std::string& error_message);
	
	      //::std::shared_ptr<enb_scheduling_info> get_scheduling_info(uint64_t bs_id);
	
	      //::std::map<int, ::std::shared_ptr<enb_scheduling_info>> scheduling_info_;
	
	// Set these values internally for now

	std::atomic<bool> central_scheduling;
	const int schedule_ahead = 0;
	bool code_pushed_;
	int prev_val_, current_val;
	
      };
      
    }
    
  }

}


#endif /* FLEXIBLE_SCHEDULER_H_ */


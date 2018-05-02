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

	void reconfigure_agent_file(int agent_id, std::string policy_name);

	void reconfigure_agent_string(int agent_id, std::string policy);

	void enable_central_scheduling(bool central_sch);

        bool apply_slice_config_policy(int agent_id, const std::string& policy,
            std::string& error_reason);
        bool remove_slice(int agent_id, const std::string& policy,
            std::string& error_reason);
        bool change_ue_slice_association(int agent_id, const std::string& policy,
            std::string& error_reason);

	static int32_t tpc_accumulated;

        int parse_enb_agent_id(const std::string& enb_agent_id_s) const;
        int get_last_agent() const;
        bool parse_rnti_imsi(int agent_id, const std::string& rnti_imsi_s,
            flexran::rib::rnti_t& rnti) const;

      private:

	void run_central_scheduler();
	void push_code(int agent_id, std::string function_name, std::string lib_name);

        void push_slice_config_reconfiguration(int agent_id,
            const protocol::flex_slice_config& slice_config, uint16_t cc_id = 0);
        void push_ue_config_reconfiguration(int agent_id,
            const protocol::flex_ue_config_reply& ue_config);
        static bool verify_dl_slice_config(const protocol::flex_dl_slice& s,
            std::string& error_message);
        static bool verify_dl_slice_removal(const protocol::flex_dl_slice& s,
            std::string& error_message);
        static bool verify_ul_slice_config(const protocol::flex_ul_slice& s,
            std::string& error_message);
        static bool verify_ul_slice_removal(const protocol::flex_ul_slice& s,
            std::string& error_message);
        bool verify_global_slice_percentage(int agent_id,
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
        bool verify_rnti_imsi(int agent_id, protocol::flex_ue_config *c,
            std::string& error_message);
	
	::std::shared_ptr<enb_scheduling_info> get_scheduling_info(int agent_id);
	
	::std::map<int, ::std::shared_ptr<enb_scheduling_info>> scheduling_info_;
	
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


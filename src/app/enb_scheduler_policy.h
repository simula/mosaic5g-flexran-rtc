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

/*! \file    enb_scheduler_policy.h
 *  \brief   app for central scheduling
 *  \authors Navid Nikaein
 *  \company Eurecom
 *  \email   navid.nikaein@eurecom.fr
 */

#ifndef ENB_SCHEDULER_POLICY_H_
#define ENB_SCHEDULER_POLICY_H_

#include "component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"
#include "rib_common.h"

#include <atomic>

namespace flexran {

  namespace app {

    namespace scheduler {

      class enb_scheduler_policy : public component {

      public:

        enb_scheduler_policy(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub), code_pushed_(false) {
	  
	  // false: the scheduler is local at the agent/eNB.
	  central_scheduling.store(false);
	  
	}

	void periodic_task();
	// in case we want to push both policy and code together 
	void push_code(uint64_t bs_id, std::string function_name, std::string lib_name);
	// change the policy here 
	void reconfigure_agent(uint64_t bs_id, std::string policy_name);
	// set the flag on  whether to delegate the scheduling to the agent or not 
	void enable_central_scheduling(bool central_sch);
	// control the transmit power control 
	static int32_t tpc_accumulated;

	void apply_policy(std::string policy_name);
	void set_policy(int rb_share);

      private:
	// where the remote scheduling is actually implemented. This is dependent on the flag set enable_central_scheduling(bool central_sch);
	void run_central_scheduler();
	
	::std::shared_ptr<enb_scheduling_info> get_scheduling_info(uint64_t bs_id);
	
	::std::map<int, ::std::shared_ptr<enb_scheduling_info>> scheduling_info_;
	
	// Set these values internally for now

	std::atomic<bool> central_scheduling;
	// this instruct the controller to apply the scheduling decesion to future SF defined by schedule_ahead variable (lookahead)
	// this implies that we have a very goo backhaul network to sustain the traffic 
	const int schedule_ahead = 0;
	bool code_pushed_;
	int prev_val_, current_val;
	
      };
      
    }
    
  }

}


#endif /* ENB_SCHEDULER_POLICY_H_ */


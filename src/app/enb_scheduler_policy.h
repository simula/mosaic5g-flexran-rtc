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

#ifndef ENB_SCHEDULER_POLICY_H_
#define ENB_SCHEDULER_POLICY_H_

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"
#include "rib_common.h"

#include <atomic>

namespace flexran {

  namespace app {

    namespace scheduler {

      class enb_scheduler_policy : public periodic_component {

      public:

	enb_scheduler_policy(rib::Rib& rib, const core::requests_manager& rm)
	  : periodic_component(rib, rm), code_pushed_(false) {
	  
	  // false: the scheduler is local at the agent/eNB.
	  central_scheduling.store(false);
	  
	}

	void run_periodic_task();
	// in case we want to push both policy and code together 
	void push_code(int agent_id, std::string function_name, std::string lib_name);
	// change the policy here 
	void reconfigure_agent(int agent_id, std::string policy_name);
	// set the flag on  whether to delegate the scheduling to the agent or not 
	void enable_central_scheduling(bool central_sch);
	// control the transmit power control 
	static int32_t tpc_accumulated;

	void apply_policy(std::string policy_name);
	void set_policy(int rb_share);

      private:
	// where the remote scheduling is actually implemented. This is dependent on the flag set enable_central_scheduling(bool central_sch);
	void run_central_scheduler();
	
	::std::shared_ptr<enb_scheduling_info> get_scheduling_info(int agent_id);
	
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


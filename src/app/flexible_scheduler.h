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
 *  \brief   app for central scheduling (not functional)
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef FLEXIBLE_SCHEDULER_H_
#define FLEXIBLE_SCHEDULER_H_

#include "component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"
#include "rib_common.h"

#include <atomic>

namespace flexran {

  namespace app {

    namespace scheduler {

      class flexible_scheduler : public component {

      public:

        flexible_scheduler(rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub), code_pushed_(false)
        {
          central_scheduling.store(false);
        }


	void periodic_task();

	//void reconfigure_agent_file(uint64_t bs_id, std::string policy_name);

	//void reconfigure_agent_string(uint64_t bs_id, std::string policy);

	//void enable_central_scheduling(bool central_sch);

	static int32_t tpc_accumulated;

      private:

	void run_central_scheduler();
	void push_code(uint64_t bs_id, std::string function_name, std::string lib_name);

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


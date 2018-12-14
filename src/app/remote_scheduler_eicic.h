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

/*! \file    remote_scheduler_eicic.h
 *  \brief   remote scheduling app for eICIC case
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef REMOTE_SCHEDULER_EICIC_H_
#define REMOTE_SCHEDULER_EICIC_H_

#include <map>

#include "component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

namespace flexran {

  namespace app {

    namespace scheduler {

      class remote_scheduler_eicic : public component {
  
      public:
	
      remote_scheduler_eicic(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
        : component(rib, rm, sub) {}
	
	void periodic_task();
	
	static int32_t tpc_accumulated;
	
      private:
	
        std::shared_ptr<enb_scheduling_info> get_scheduling_info(uint64_t bs_id);
	
        std::map<uint64_t, std::shared_ptr<enb_scheduling_info>> scheduling_info_;
	
	// Set these values internally for now
	
	const int schedule_ahead = 1;
	
	int abs_[10] = {0, 1, 0, 1, 0, 0, 0, 1, 0, 1};

        const uint64_t macro_bs_id_ = 0;

      };

    }

  }

}
#endif

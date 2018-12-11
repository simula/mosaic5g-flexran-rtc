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

/*! \file    remote_scheduler_helper.h
 *  \brief   helper file for remote scheduling app
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef REMOTE_SCHEDULER_HELPER_H_
#define REMOTE_SCHEDULER_HELPER_H_

#include "flexran.pb.h"
#include "rib_common.h"

#include "component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

namespace flexran {

  namespace app {

    namespace scheduler {

      class remote_scheduler_helper {

      private:
	
	struct ue_stats {
	  
	  int ue_id;
	  int bytes_in_ccch;
	  int total_bytes_in_buffers;
	  int hol_delay;
	  int cqi;
	  int harq_round;

	ue_stats():ue_id(0),bytes_in_ccch(0),total_bytes_in_buffers(0),hol_delay(0),cqi(0),harq_round(0){}
	  
	};

	static bool compare_stats(const ue_stats& a, const ue_stats& b);
	
      public:
      
	static void run_dlsch_scheduler_preprocessor(const protocol::flex_cell_config& cell_config,
						     const protocol::flex_ue_config_reply& ue_configs,
						     const protocol::flex_lc_config_reply& lc_configs,
						     ::std::shared_ptr<rib::enb_rib_info> agent_config,
						     ::std::shared_ptr<enb_scheduling_info> sched_info,
						     rib::frame_t frame,
						     rib::subframe_t subframe);
      
	static void assign_rbs_required(::std::shared_ptr<ue_scheduling_info> ue_sched_info,
					::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info,
					const protocol::flex_cell_config& cell_config,
					const protocol::flex_lc_ue_config& lc_ue_config);
	
	static void perform_pre_processor_allocation(const protocol::flex_cell_config& cell_config,
						     std::shared_ptr<enb_scheduling_info> sched_info,
						     std::shared_ptr<ue_scheduling_info> ue_sched_info,
						     int transmission_mode);


	static const std::shared_ptr<std::vector<int>> sort_UEs(const protocol::flex_cell_config& cell_config,
								const protocol::flex_ue_config_reply& ue_configs,
								const ::std::shared_ptr<rib::enb_rib_info> agent_config); 
	  	
      };
    }

  }

}

#endif

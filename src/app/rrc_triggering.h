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

/*! \file    rrc_triggering.h
 *  \brief   trigger RRC measurements at the agent
 *  \authors Shahab SHARIAT BAGHERI
 *  \company Eurecom
 *  \email   shahab.shariat@eurecom.fr
 */

#ifndef RRC_TRIGGERING_H_
#define RRC_TRIGGERING_H_

#include "component.h"
// #include "enb_scheduling_info.h"
// #include "ue_scheduling_info.h"
#include "rib_common.h"

#include <atomic>

namespace flexran {

  namespace app {

    namespace rrc {

      class rrc_triggering : public component {

      public:

        rrc_triggering(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub) {
	    
	}

	void periodic_task();

	// void push_code(uint64_t bs_id, std::string function_name, std::string lib_name);

	void reconfigure_agent(uint64_t bs_id, std::string freq_measure);

	void enable_rrc_triggering(std::string freq_measure);
	

	
      };
      
    }
    
  }

}


#endif /* RRC_TRIGGERING_H_ */

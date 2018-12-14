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

/*! \file    delegation_manager.h
 *  \brief   example app for control delegation
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef DELEGATION_MANAGER_H_
#define DELEGATION_MANAGER_H_

#include "component.h"
#include "rib_common.h"

namespace flexran {
  
  namespace app {

    namespace management {

      class delegation_manager : public component {
	
      public:

        delegation_manager(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub), delegation_steps_{false} {}

	void periodic_task();

        void push_code(uint64_t bs_id, std::string function_name, std::string lib_name);

        void reconfigure_agent(uint64_t bs_id, std::string policy_name);
	
      private:
	bool delegation_steps_[6];
      };

    }
  }
}

#endif

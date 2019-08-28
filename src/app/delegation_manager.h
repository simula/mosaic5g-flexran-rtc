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
 *  \brief   application for control delegation (send shared objects to agent)
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef DELEGATION_MANAGER_H_
#define DELEGATION_MANAGER_H_

#include "component.h"

namespace flexran {
  
  namespace app {

    namespace management {

      class delegation_manager : public component {
	
      public:

        delegation_manager(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub) {}

        bool push_object(const std::string& bs, const std::string& name,
                         const char *data, int len, std::string& error_reason);

      private:
        void push_code(uint64_t bs_id, const std::string& name,
                       const char *data, int len);
      };
    }
  }
}

#endif

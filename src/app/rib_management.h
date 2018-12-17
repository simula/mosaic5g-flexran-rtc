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

/*! \file    rib_management.h
 *  \brief   app polling BSs for connection management purposes
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef RIB_MANAGEMENT_H_
#define RIB_MANAGEMENT_H_

#include "component.h"
#include "rib_common.h"

namespace flexran {

  namespace app {

    namespace management {

      class rib_management : public component {

      public:

        rib_management(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub);
        void tick(uint64_t ms);

      private:
        std::set<uint64_t> inactive_bs_;

        void send_enb_config_request(uint64_t bs_id);
        void send_ue_config_request(uint64_t bs_id);
        void send_lc_config_request(uint64_t bs_id);
      };
    }
  }
}

#endif /* RIB_MANAGEMENT_H_ */

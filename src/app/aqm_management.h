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

/*! \file    aqm_management.h
 *  \brief   handle Active Queue Management
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef AQM_MANAGEMENT_H_
#define AQM_MANAGEMENT_H_

#include "component.h"
#include "rib_common.h"

namespace flexran {

  namespace app {

    namespace management {

      class aqm_management : public component {

      public:

        aqm_management(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub);

        bool handle_aqm_config(const std::string& bs, const std::string& policy,
            std::string& error_reason);
        bool handle_aqm_config_rnti(const std::string& bs, const std::string& ue,
            const std::string& policy, std::string& error_reason);

      private:

        void send_lc_config_update(uint64_t bs_id, protocol::flex_lc_config_reply lc_config);
      };
    }
  }
}

#endif /* AQM_MANAGEMENT_H_ */

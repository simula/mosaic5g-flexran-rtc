/*
 * Copyright 2016-2020 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    plmn_management.h
 *  \brief   app manages a BS's PLMN and MME/control CN node
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef PLMN_MANAGEMENT_H_
#define PLMN_MANAGEMENT_H_

#include "component.h"
#include "rib_common.h"

#include <atomic>

namespace flexran {

  namespace app {

    namespace management {

      class plmn_management : public component {

      public:

        plmn_management(rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub);
        void add_mme(const std::string& bs, const std::string& config);
        void remove_mme(const std::string& bs, const std::string& config);

      private:

        void push_mme_config(uint64_t bs_id, const protocol::flex_s1ap_config& s1ap);
      };
    }
  }
}

#endif /* PLMN_MANAGEMENT_H_ */

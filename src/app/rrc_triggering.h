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
 *  \brief   trigger RRC measurements
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef RRC_TRIGGERING_H_
#define RRC_TRIGGERING_H_

#include "component.h"
#include "rib_common.h"

#include <atomic>
#include <unordered_set>
#include <map>

namespace flexran {

  namespace app {

    namespace rrc {

      class rrc_triggering : public component {

      public:

        rrc_triggering(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub);

        bool rrc_reconf(const std::string& bs, const std::string& policy,
            std::string& error_reason);
        bool rrc_ho(const std::string& sbs, const std::string& ue, const std::string& tbs,
                    std::string& error_reason);
        bool rrc_x2_ho_net_control(const std::string& bs, bool x2_ho_net_control,
            std::string& error_reason);

        void bs_added(uint64_t bs_id);
        void bs_removed(uint64_t bs_id);

      private:
        uint64_t parse_bs_agent_id(const std::string& s) const;
        uint64_t parse_bs_id(const std::string& s) const;
        uint64_t parse_physical_cell_id(const std::string& s) const;

        void push_config(uint64_t bs_id, const protocol::flex_measurement_info& rrc_info);
        void push_ho(uint64_t bs_id, flexran::rib::rnti_t rnti, uint32_t target_phy_cell_id);
        void push_x2_ho_net_control(uint64_t bs_id, bool x2_ho_net_control);

        std::unordered_set<uint64_t> set_check_phyCellId;
        std::map<uint64_t, int> map_phyCellId;
        bs2::connection tick_check_phyCellId;
        void check_phyCellId(uint64_t tick);
      };
    }
  }
}

#endif /* RRC_TRIGGERING_H_ */

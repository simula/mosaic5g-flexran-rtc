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

/*! \file    cell_mac_rib_info.h
 *  \brief   wrapper class for a cell's statistics
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef CELL_MAC_RIB_INFO_H_
#define CELL_MAC_RIB_INFO_H_

#include <cstdint>

#include "flexran.pb.h"

namespace flexran {

  namespace rib {

    class cell_mac_rib_info {
    public:

      void update_cell_stats_report(const protocol::flex_cell_stats_report& stats_report);

      const protocol::flex_cell_stats_report& get_cell_stats_report() const {
        return cell_stats_report_;
      }
  
    private:
      protocol::flex_cell_stats_report cell_stats_report_;

    };

  }

}

#endif

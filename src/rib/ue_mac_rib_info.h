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

/*! \file    ue_mac_rib_info.h
 *  \brief   wrapper class for a UE's configuration
 *  \authors Xenofon Foukas, Navid Nikaein, Shahab SHARIAT BAGHERI, Robert
 *           Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           shahab.shariat@eurecom.fr, robert.schmidt@eurecom.fr
 */

#ifndef UE_MAC_RIB_INFO_H_
#define UE_MAC_RIB_INFO_H_

#include <cstdint>
#include <mutex>

#include "rib_common.h"
#include "flexran.pb.h"

template <class T, size_t rows, size_t cols>
using array2d = std::array<std::array<T, cols>, rows>;

template <class T, size_t rows, size_t cols, size_t d>
using array3d = std::array<std::array<std::array<T, d>, cols>, rows>;

namespace flexran {

  namespace rib {

    class ue_mac_rib_info {
      
    public:
      
    ue_mac_rib_info(rnti_t rnti)
      : rnti_(rnti), harq_stats_{{{protocol::FLHS_ACK}}},
	uplink_reception_stats_{0}, ul_reception_data_{{0}} {

	  for (int i = 0; i < MAX_NUM_CC; i++) {
	    for (int j = 0; j < MAX_NUM_HARQ; j++) {
	      for (int k = 0; k < MAX_NUM_TB; k++) {
		active_harq_[i][j][k] = true;
	      }
	    }
	  }
	}

     void update_dl_sf_info(const protocol::flex_dl_info& dl_info);

     void update_ul_sf_info(const protocol::flex_ul_info& ul_info);

     void update_mac_stats_report(const protocol::flex_ue_stats_report& stats_report);
     
     void dump_stats() const;

     std::string dump_stats_to_string() const;

     std::string dump_stats_to_json_string() const;

     static std::string format_stats_to_json(rnti_t rnti,
                                             const std::string& mac_stats,
                                             const std::array<std::string, 8>& harq);

     //! Access is only safe when the RIB is not active, i.e. within apps
     const protocol::flex_ue_stats_report& get_mac_stats_report() const { return mac_stats_report_; }
     
     uint8_t get_harq_stats(uint16_t cell_id, int harq_pid) const {
       return harq_stats_[cell_id][harq_pid][0];
     }
     
     //! Access is only safe when the RIB is not active, i.e. within apps
     const array3d<uint8_t, MAX_NUM_CC, MAX_NUM_HARQ, MAX_NUM_TB>& get_all_harq_stats() const {
       return harq_stats_;
     }

     int get_next_available_harq(uint16_t cell_id) const {
       for (int i = 0; i < MAX_NUM_HARQ; i++) {
	 if (active_harq_[cell_id][i][0] == true) {
	   return i;
	 }
       }
       return -1;
     }

     bool has_available_harq(uint16_t cell_id) const {
       for (int i = 0; i < MAX_NUM_HARQ; i++) {
	 if (active_harq_[cell_id][i][0] == true) {
	   return true;
	 }
       }
       return false;
     }
     
     void harq_scheduled(uint16_t cell_id, uint8_t harq_pid) {
       active_harq_[cell_id][harq_pid][0] = false;
       active_harq_[cell_id][harq_pid][1] = false;
     }
     
    private:
     
     rnti_t rnti_;
     
     protocol::flex_ue_stats_report mac_stats_report_;
     mutable std::mutex mac_stats_report_mutex_;

     // TODO this could/should be protected with mutexes, too
     // SF info
     array3d<uint8_t, MAX_NUM_CC, MAX_NUM_HARQ, MAX_NUM_TB> harq_stats_;
     array3d<bool, MAX_NUM_CC, MAX_NUM_HARQ, MAX_NUM_TB> active_harq_;
     std::array<uint8_t, MAX_NUM_CC> uplink_reception_stats_;
     array2d<uint8_t, MAX_NUM_CC, MAX_NUM_LC> ul_reception_data_;
     uint8_t tpc_;
    };

  }

}

#endif

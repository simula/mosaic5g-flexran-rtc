/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
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
     protocol::flex_ue_stats_report& get_mac_stats_report() { return mac_stats_report_; }
     
     uint8_t get_harq_stats(uint16_t cell_id, int harq_pid) {
       return harq_stats_[cell_id][harq_pid][0];
     }
     
     //! Access is only safe when the RIB is not active, i.e. within apps
     array3d<uint8_t, MAX_NUM_CC, MAX_NUM_HARQ, MAX_NUM_TB>& get_all_harq_stats() {
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

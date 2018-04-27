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

/*! \file    enb_scheduling_info.h
 *  \brief   information for central scheduling
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef ENB_SCHEDULING_INFO_H_
#define ENB_SCHEDULING_INFO_H_

#include <map>
#include <memory>

#include "ue_scheduling_info.h"
#include "rib_common.h"
#include "flexran.pb.h"

namespace flexran {

  namespace app {

    namespace scheduler {

      class enb_scheduling_info {
	
      public:
	
	rib::frame_t get_last_checked_frame() const { return last_checked_frame_; }
	
	rib::subframe_t get_last_checked_subframe() const { return last_checked_subframe_; }
	
	void set_last_checked_frame(rib::frame_t frame) { last_checked_frame_ = frame; }
	
	void set_last_checked_subframe(rib::subframe_t subframe) { last_checked_subframe_ = subframe; }
	
	void create_ue_scheduling_info(rib::rnti_t rnti);
	
	uint8_t* get_vrb_map(uint16_t cell_id) {
	  return vrb_map_[cell_id];
	}

	void toggle_vrb_map(uint16_t cell_id, uint16_t index) {
	  vrb_map_[cell_id][index] = 1;
	}
	
	uint8_t get_num_pdcch_symbols(uint16_t cell_id) const { return num_pdcch_symbols_[cell_id]; }

	uint32_t get_nCCE_rem(uint16_t cell_id) const { return nCCE_rem_[cell_id]; }
	
	void assign_CCE(uint16_t cell_id, int nCCE) { nCCE_rem_[cell_id] -= nCCE; }

	void remove_CCE(uint16_t cell_id, int nCCE) {nCCE_rem_[cell_id] += nCCE; }
	
	void increase_num_pdcch_symbols(const protocol::flex_cell_config& cell_config,
					flexran::rib::subframe_t subframe);
	
	void start_new_scheduling_round(rib::subframe_t subframe,
					const protocol::flex_cell_config& cell_config);
	
	::std::shared_ptr<ue_scheduling_info> get_ue_scheduling_info(rib::rnti_t rnti);
	
      private:
	
	rib::frame_t last_checked_frame_;
	rib::subframe_t last_checked_subframe_;
	
	::std::map<rib::rnti_t, ::std::shared_ptr<ue_scheduling_info>> scheduling_info_;
	
	uint8_t vrb_map_[rib::MAX_NUM_CC][rib::N_RBG_MAX] = {{0}};
	uint8_t num_pdcch_symbols_[rib::MAX_NUM_CC] = {0};

	uint32_t nCCE_rem_[rib::MAX_NUM_CC] = {0};
      };

    }

  }

}
      
#endif

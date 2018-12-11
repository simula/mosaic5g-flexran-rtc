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

/*! \file    remote_scheduler_primitives.h
 *  \brief   helper providing scheduling primitives
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef REMOTE_SCHEDULER_PRIMITIVES_H_
#define REMOTE_SCHEDULER_PRIMITIVES_H_

#include "flexran.pb.h"
#include "rib_common.h"

#include "component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"
#include "rt_controller_common.h"


namespace flexran {

  namespace app {

    namespace scheduler {

#ifndef ABSTRACT_SIMULATION
      const static int target_dl_mcs_ = 28;
#else
      const static int target_dl_mcs_ = 15;
#endif

      
      bool needs_scheduling(::std::shared_ptr<enb_scheduling_info>& enb_sched_info,
			    rib::frame_t curr_frame,
			    rib::subframe_t curr_subframe);
      
      bool CCE_allocation_infeasible(::std::shared_ptr<enb_scheduling_info>& enb_sched_info,
				     const protocol::flex_cell_config& cell_config,
				     const protocol::flex_ue_config& ue_config,
				     uint8_t aggregation,
				     rib::subframe_t subframe);
      
      uint16_t get_min_rb_unit(const protocol::flex_cell_config& cell_config);
      
      uint16_t get_nb_rbg(const protocol::flex_cell_config& cell_config);
      
      uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb);
      
      unsigned char get_I_TBS(unsigned char I_MCS);
      
      uint8_t get_mi(const protocol::flex_cell_config& cell_config,
		     rib::subframe_t subframe);
      
      uint16_t get_nCCE_max(uint8_t num_pdcch_symbols,
			    const protocol::flex_cell_config& cell_config,
			    rib::subframe_t subframe);
      
      uint16_t get_nquad(uint8_t num_pdcch_symbols,
			 const protocol::flex_cell_config& cell_config,
			 uint8_t mi);
      
      int get_nCCE_offset(const uint8_t aggregation,
			  const int nCCE,
			  const int common_dci,
			  const rib::rnti_t rnti,
			  const rib::subframe_t subframe);
      
      uint8_t get_phich_resource(const protocol::flex_cell_config& cell_config);

      uint32_t allocate_prbs_sub(int nb_rb,
				 uint8_t *rballoc,
				 const protocol::flex_cell_config& cell_config);

      uint8_t get_aggregation(uint8_t bw_index, uint8_t cqi, protocol::flex_dci_format dci_fmt);

      int get_bw_index(int n_rb_dl);
      
    }

  }

}

#endif

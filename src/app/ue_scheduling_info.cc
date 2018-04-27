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

/*! \file    ue_scheduling_info.cc
 *  \brief   UE scheduling info helper
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include <algorithm>
#include <iostream>

#include "ue_scheduling_info.h"

void flexran::app::scheduler::ue_scheduling_info::start_new_scheduling_round() {
  std::fill( &rballoc_sub_[0][0], &rballoc_sub_[0][0] + sizeof(rballoc_sub_) /* / sizeof(flags[0][0]) */, 0 );
  std::fill(nb_rbs_required_, nb_rbs_required_ + rib::MAX_NUM_CC, 0);
  std::fill(nb_rbs_required_remaining_, nb_rbs_required_remaining_ + rib::MAX_NUM_CC, 0);
  std::fill(nb_rbs_required_remaining_1_, nb_rbs_required_remaining_1_ + rib::MAX_NUM_CC, 0);
  std::fill(pre_nb_rbs_available_, pre_nb_rbs_available_ + rib::MAX_NUM_CC, 0);
  std::fill(&rballoc_sub_scheduled_[0][0][0], &rballoc_sub_scheduled_[0][0][0] + sizeof(rballoc_sub_scheduled_), 0);  
}

int flexran::app::scheduler::ue_scheduling_info::get_harq_round(uint16_t cell_id, int harq_pid) const {
  if (cell_id < rib::MAX_NUM_CC && harq_pid < rib::MAX_NUM_HARQ) {
     return harq_round_[cell_id][harq_pid][0];
   }
   return -1;
}


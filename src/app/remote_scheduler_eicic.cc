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

/*! \file    remote_scheduler_eicic.cc
 *  \brief   remote scheduling app for eICIC case
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include <iostream>

#include "remote_scheduler_eicic.h"
#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"
#include "flexran.pb.h"
#include "rib_common.h"
#include "cell_mac_rib_info.h"
#include "flexran_log.h"

int32_t flexran::app::scheduler::remote_scheduler_eicic::tpc_accumulated = 0;

void flexran::app::scheduler::remote_scheduler_eicic::periodic_task() {

  rib::frame_t target_frame;
  rib::subframe_t target_subframe;
  
  //unsigned char aggregation;
  //uint16_t total_nb_available_rb[rib::MAX_NUM_CC];

  //uint16_t nb_available_rb, nb_rb, nb_rb_tmp, TBS, sdu_length_total = 0;
  //uint8_t harq_pid, round, ta_len = 0;

  //uint32_t dci_tbs;
  //int mcs, ndi, tpc = 1, mcs_tmp;
  //uint32_t ce_flags = 0;
  //uint32_t data_to_request;

  //uint8_t header_len_dcch = 0, header_len_dcch_tmp = 0, header_len_dtch = 0, header_len_dtch_tmp = 0;
  //uint8_t header_len = 0, header_len_tmp = 0;

  //bool ue_has_transmission = false;

  bool schedule_macro = true;
  
  bool schedule_flag = false;

  std::set<uint64_t> bs_ids = rib_.get_available_base_stations();

  // Check if scheduling needs to be performed and who needs to be scheduled (macro or pico cells)
  for (uint64_t bs_id : bs_ids) {
    ::std::shared_ptr<const rib::enb_rib_info> bs_config = rib_.get_bs(bs_id);
    const protocol::flex_enb_config_reply& enb_config = bs_config->get_enb_config();
    const protocol::flex_ue_config_reply& ue_configs = bs_config->get_ue_configs();
    //const protocol::flex_lc_config_reply& lc_configs = bs_config->get_lc_configs();

    rib::frame_t current_frame = bs_config->get_current_frame();
    rib::subframe_t current_subframe = bs_config->get_current_subframe();

    // Check if scheduling context for this eNB is already present and if not create it
    ::std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(bs_id);
    if (enb_sched_info) {
      // Nothing to do if this exists
    } else { // eNB sched info was not found for this BS
      LOG4CXX_INFO(flog::app, "Config was not found. Creating");
      scheduling_info_.insert(::std::pair<int,
			      ::std::shared_ptr<enb_scheduling_info>>(bs_id,
								      ::std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
      enb_sched_info = get_scheduling_info(bs_id);
    }
    
    LOG4CXX_DEBUG(flog::app, "Checking id " << bs_id);
    LOG4CXX_DEBUG(flog::app, "Current subframe " << current_subframe);

    // Check if we have already run the scheduler for this particular time slot and if yes go to next eNB
    if (!needs_scheduling(enb_sched_info, current_frame, current_subframe)) {
      continue;
    }
    
    enb_sched_info->set_last_checked_frame(current_frame);
    enb_sched_info->set_last_checked_subframe(current_subframe);

    target_subframe = (current_subframe + schedule_ahead) % 10;
    if (target_subframe < current_subframe) {
      target_frame = (current_frame + 1) % 1024;
    } else {
      target_frame = current_frame;
    }
    int additional_frames = schedule_ahead / 10;
    target_frame = (current_frame + additional_frames) % 1024;
    
    if (abs_[target_subframe] != 0) {
      schedule_flag = true;
    }
    
    // Only need to check if scheduling needs to be performed if this is a pico cell
    if (bs_id == macro_bs_id_) {
      continue;
    }
   
    // Go through the cells and schedule the UEs of this cell
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      const protocol::flex_cell_config cell_config = enb_config.cell_config(i);
      uint32_t cell_id = cell_config.cell_id();

      for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
	const protocol::flex_ue_config ue_config = ue_configs.ue_config(UE_id);

	if (ue_config.pcell_carrier_index() == cell_id) {
	  
	  // Get the MAC stats for this UE
	  ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = bs_config->get_ue_mac_info(ue_config.rnti());
	
	  LOG4CXX_DEBUG(flog::app, "Got the MAC stats of the UE with rnti: " << ue_config.rnti());

	  if (!ue_mac_info) {
	    continue;
	  }
	  
	  const protocol::flex_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();
	  
	  for (int j = 0; j < mac_report.rlc_report_size(); j++) {
	    // If there is something to transmit in one of the pico cells, set the macro cell scheduling flag to false
	    if (mac_report.rlc_report(j).tx_queue_size() > 0) {
	      LOG4CXX_DEBUG(flog::app, "We need to schedule the pico cell" << target_subframe);
	      schedule_macro = false;
	    }
	  }
	}
      }
    }
  }

  if (!schedule_flag) {
    return;
  }

  for (uint64_t bs_id : bs_ids) {
    
    // Check the BS id and the macro cell schedule flag and choose if this BS will be scheduled or not
    if (((bs_id == macro_bs_id_) && (!schedule_macro)) ||
        ((bs_id != macro_bs_id_) && (schedule_macro))) {
      continue;
    } else { //Simply send an empty dl_mac_config message to notify the permission to schedule
      protocol::flexran_message out_message;

      // Create dl_mac_config message header
      protocol::flex_header *header(new protocol::flex_header);
      header->set_type(protocol::FLPT_DL_MAC_CONFIG);
      header->set_version(0);
      header->set_xid(0);
    
      std::shared_ptr<rib::enb_rib_info> bs_config = rib_.get_bs(bs_id);
      //const protocol::flex_enb_config_reply& enb_config = bs_config->get_enb_config();
      //const protocol::flex_ue_config_reply& ue_configs = bs_config->get_ue_configs();
      //const protocol::flex_lc_config_reply& lc_configs = bs_config->get_lc_configs();
      
      rib::frame_t current_frame = bs_config->get_current_frame();
      rib::subframe_t current_subframe = bs_config->get_current_subframe();
      
      // Check if scheduling context for this eNB is already present and if not create it
      ::std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(bs_id);
      if (enb_sched_info) {
	// Nothing to do if this exists
      } else { // eNB sched info was not found for this BS
        LOG4CXX_INFO(flog::app, "Config was not found. Creating");
	scheduling_info_.insert(::std::pair<int,
				::std::shared_ptr<enb_scheduling_info>>(bs_id,
									::std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
	enb_sched_info = get_scheduling_info(bs_id);
      }
      
      target_subframe = (current_subframe + schedule_ahead) % 10;
      if (target_subframe < current_subframe) {
	target_frame = (current_frame + 1) % 1024;
      } else {
	target_frame = current_frame;
      }
      int additional_frames = schedule_ahead / 10;
      target_frame = (current_frame + additional_frames) % 1024;
      
      // Create dl_mac_config message
      protocol::flex_dl_mac_config *dl_mac_config_msg(new protocol::flex_dl_mac_config);
      dl_mac_config_msg->set_allocated_header(header);
      dl_mac_config_msg->set_sfn_sf(rib::get_sfn_sf(target_frame, target_subframe));
      
      // Create and send the flexran message
      out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
      out_message.set_allocated_dl_mac_config_msg(dl_mac_config_msg);
      req_manager_.send_message(bs_id, out_message);
      
    }  
  }
}   

std::shared_ptr<flexran::app::scheduler::enb_scheduling_info>
flexran::app::scheduler::remote_scheduler_eicic::get_scheduling_info(uint64_t bs_id) {
  auto it = scheduling_info_.find(bs_id);
  if (it != scheduling_info_.end()) {
    return it->second;
  }
  return ::std::shared_ptr<enb_scheduling_info>(nullptr);
}

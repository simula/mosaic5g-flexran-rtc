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

/*! \file    flexible_scheduler.cc
 *  \brief   app for central scheduling and RRM calls helper
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#include <iostream>
#include <string>
#include <streambuf>
#include <fstream>
#include <map>

#include <google/protobuf/util/json_util.h>

#include "flexible_scheduler.h"
#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"
#include "flexran.pb.h"
#include "rib_common.h"
#include "cell_mac_rib_info.h"
#include "band_check.h"

#include "flexran_log.h"

int32_t flexran::app::scheduler::flexible_scheduler::tpc_accumulated = 0;

void flexran::app::scheduler::flexible_scheduler::periodic_task() {

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());
  
  for (auto& agent_id : agent_ids) {
    
    if (!code_pushed_) {
      std::string path = "";
      std::string remote_sched = "";
      std::string default_sched = "";
      
      if(const char* env_p = std::getenv("FLEXRAN_RTC_HOME")) {
	path = path + env_p + "/tests/delegation_control/";
      } else {
	path = "../tests/delegation_control/";
      }

      remote_sched = path + "libremote_sched.so";
      default_sched = path + "libdefault_sched.so";
      
      push_code(agent_id, "flexran_schedule_ue_spec_remote", remote_sched);
      push_code(agent_id, "flexran_schedule_ue_spec_default", default_sched); 
      
      code_pushed_ = true;
    }
  } 
  
  if (central_scheduling.load() == true) {
    run_central_scheduler();
  } else {
    return;
  }
}

bool flexran::app::scheduler::flexible_scheduler::apply_slice_config_policy(
    int agent_id, const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_agent(agent_id)) {
    error_reason = "Agent does not exist";
    LOG4CXX_ERROR(flog::app, "Agent " << agent_id << " does not exist");
    return false;
  }

  protocol::flex_slice_config slice_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &slice_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf slice_config message:" << ret.ToString());
    return false;
  }

  // enforce every DL/UL slice has an ID and well formed parameters
  for (int i = 0; i < slice_config.dl_size(); i++) {
    if (!verify_dl_slice_config(slice_config.dl(i), error_reason)) {
      error_reason += " in DL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }
  for (int i = 0; i < slice_config.ul_size(); i++) {
    if (!verify_ul_slice_config(slice_config.ul(i), error_reason)) {
      error_reason += " in UL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  // enforce that the sum percentage is equal or below 100 percent
  if (!verify_global_slice_percentage(agent_id, slice_config, error_reason)) {
    LOG4CXX_ERROR(flog::app, error_reason);
    return false;
  }

  // no UL slice is allowed to have the same firstRb as any other (in fact,
  // together with the percentage value it is computed that they don't
  // overlap). Therefore, if we have one new slice without a firstRb value, try
  // to add firstRb by checking a "free" region. This is to keep the short
  // version of the slice configuration end point working and we therefore
  // check that this slice carries nothing except an ID */
  if (slice_config.ul_size() == 1
      && slice_config.ul(0).id() != 0
      && !slice_config.ul(0).has_label()
      && !slice_config.ul(0).has_percentage()
      && !slice_config.ul(0).has_isolation()
      && !slice_config.ul(0).has_priority()
      && !slice_config.ul(0).has_first_rb()
      && !slice_config.ul(0).has_maxmcs()
      && slice_config.ul(0).sorting_size() == 0
      && !slice_config.ul(0).has_accounting()
      && !slice_config.ul(0).has_scheduler_name()
      && try_add_first_rb(agent_id, *slice_config.mutable_ul(0)))
    LOG4CXX_WARN(flog::app, "no firstRb value detected, added "
        << slice_config.ul(0).first_rb() << " so that it does not clash in "
        << "the agent. You can override this by specifying a firstRb value.");

  protocol::flex_cell_config cell_config;
  cell_config.mutable_slice_config()->CopyFrom(slice_config);
  push_cell_config_reconfiguration(agent_id, cell_config);
  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(slice_config, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent new configuration to agent " << agent_id
      << ":\n" << pol_corrected);

  return true;
}

bool flexran::app::scheduler::flexible_scheduler::remove_slice(int agent_id,
    const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_agent(agent_id)) {
    error_reason = "Agent does not exist";
    LOG4CXX_ERROR(flog::app, "Agent " << agent_id << " does not exist");
    return false;
  }

  protocol::flex_slice_config slice_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &slice_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf slice_config message:" << ret.ToString());
    return false;
  }

  // enforce every DL/UL slice has an ID and well formed parameters
  for (int i = 0; i < slice_config.dl_size(); i++) {
    if (!verify_dl_slice_removal(slice_config.dl(i), error_reason)) {
      error_reason += " in DL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (!rib_.get_agent(agent_id)->has_dl_slice(slice_config.dl(i).id())) {
      error_reason = "DL slice " + std::to_string(slice_config.dl(i).id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }
  for (int i = 0; i < slice_config.ul_size(); i++) {
    if (!verify_ul_slice_removal(slice_config.ul(i), error_reason)) {
      error_reason += " in UL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (!rib_.get_agent(agent_id)->has_ul_slice(slice_config.ul(i).id())) {
      error_reason = "UL slice " + std::to_string(slice_config.ul(i).id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  protocol::flex_cell_config cell_config;
  cell_config.mutable_slice_config()->CopyFrom(slice_config);
  push_cell_config_reconfiguration(agent_id, cell_config);
  LOG4CXX_INFO(flog::app, "sent remove slice command to agent " << agent_id
      << ":\n" << policy << "\n");

  return true;
}

bool flexran::app::scheduler::flexible_scheduler::change_ue_slice_association(
    int agent_id, const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_agent(agent_id)) {
    error_reason = "Agent does not exist";
    LOG4CXX_ERROR(flog::app, "Agent " << agent_id << " does not exist");
    return false;
  }

  protocol::flex_ue_config_reply ue_config_reply;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &ue_config_reply,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf ue_config_reply message:" << ret.ToString());
    return false;
  }

  // enforce UE configaration
  if (ue_config_reply.ue_config_size() == 0) {
    error_reason = "Missing UE configuration";
    LOG4CXX_ERROR(flog::app,
        "the ue_config_reply message must contain a UE configuration");
    return false;
  }
  // enforce UE configuration has both RNTI and UL or DL slice ID
  for (int i = 0; i < ue_config_reply.ue_config_size(); i++) {
    if (!verify_ue_slice_assoc_msg(ue_config_reply.ue_config(i), error_reason)) {
      error_reason += " in UE-slice association at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (ue_config_reply.ue_config(i).has_dl_slice_id()
        && !rib_.get_agent(agent_id)->has_dl_slice(ue_config_reply.ue_config(i).dl_slice_id())) {
      error_reason = "DL slice "
          + std::to_string(ue_config_reply.ue_config(i).dl_slice_id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (ue_config_reply.ue_config(i).has_ul_slice_id()
        && !rib_.get_agent(agent_id)->has_ul_slice(ue_config_reply.ue_config(i).ul_slice_id())) {
      error_reason = "UL slice "
          + std::to_string(ue_config_reply.ue_config(i).ul_slice_id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  for (int i = 0; i < ue_config_reply.ue_config_size(); i++) {
    if (!verify_rnti_imsi(agent_id, ue_config_reply.mutable_ue_config(i), error_reason)) {
      error_reason += " in UE-slice association at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  push_ue_config_reconfiguration(agent_id, ue_config_reply);
  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(ue_config_reply, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent new UE configuration to agent "
      << agent_id << ":\n" << pol_corrected);

  return true;
}

bool flexran::app::scheduler::flexible_scheduler::apply_cell_config_policy(
    int agent_id, const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_agent(agent_id)) {
    error_reason = "Agent does not exist";
    LOG4CXX_ERROR(flog::app, "Agent " << agent_id << " does not exist");
    return false;
  }

  protocol::flex_cell_config cell_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &cell_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf ue_config_reply message:" << ret.ToString());
    return false;
  }

  if (!verify_cell_config_for_restart(cell_config, error_reason)) {
    LOG4CXX_ERROR(flog::app, error_reason);
    return false;
  }

  push_cell_config_reconfiguration(agent_id, cell_config);
  LOG4CXX_INFO(flog::app, "sent new cell configuration to agent " << agent_id
      << ":\n" << policy << "\n");

  return true;
}

void flexran::app::scheduler::flexible_scheduler::reconfigure_agent_file(int agent_id, std::string policy_name) {
  std::ifstream policy_file(policy_name);
  std::string str_policy;
  int len;

  if (!policy_file.good()) {
    LOG4CXX_WARN(flog::app, "The policy could not be loaded");
    return;
  }
  
  policy_file.seekg(0, std::ios::end);
  len = policy_file.tellg();
  if (len <= 0) {
    LOG4CXX_WARN(flog::app, "Policy could not be found. Make sure that it is stored in the proper directory");
    return;
  }
  str_policy.reserve(len);
  policy_file.seekg(0, std::ios::beg);

  str_policy.assign((std::istreambuf_iterator<char>(policy_file)),
		    std::istreambuf_iterator<char>());

  reconfigure_agent_string(agent_id, str_policy);
  
}

void flexran::app::scheduler::flexible_scheduler::reconfigure_agent_string(int agent_id, std::string policy) {

  protocol::flexran_message config_message;
  // Create control delegation message header
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);
  
  protocol::flex_agent_reconfiguration *agent_reconfiguration_msg(new protocol::flex_agent_reconfiguration);
  agent_reconfiguration_msg->set_allocated_header(config_header);

  agent_reconfiguration_msg->set_policy(policy);

  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_agent_reconfiguration_msg(agent_reconfiguration_msg);
  req_manager_.send_message(agent_id, config_message);
}

void flexran::app::scheduler::flexible_scheduler::push_code(int agent_id, std::string function_name, std::string lib_name) {
  protocol::flexran_message d_message;
  // Create control delegation message header
  protocol::flex_header *delegation_header(new protocol::flex_header);
  delegation_header->set_type(protocol::FLPT_DELEGATE_CONTROL);
  delegation_header->set_version(0);
  delegation_header->set_xid(0);
  
  protocol::flex_control_delegation *control_delegation_msg(new protocol::flex_control_delegation);
  control_delegation_msg->set_allocated_header(delegation_header);
  control_delegation_msg->set_delegation_type(protocol::FLCDT_MAC_DL_UE_SCHEDULER);
  
  ::std::ifstream fin(lib_name, std::ios::in | std::ios::binary);
  if (!fin.good()) {
    // TODO: Need to log this properly
    LOG4CXX_WARN(flog::app, "The library could not be loaded");
    return;
  }
  fin.seekg(0, std::ios::end );  
  int len = fin.tellg();
  if (len <= 0) {
    LOG4CXX_WARN(flog::app, "Library could not be found. Make sure that it is stored in the proper directory");
    return;
  }
  char ret[len];
  fin.seekg(0, std::ios::beg);   
  fin.read(ret, len);  
  fin.close();
  std::string test(ret, len);
  control_delegation_msg->set_payload(ret, len);
  control_delegation_msg->set_name(function_name);
  // Create and send the progran message
  d_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  d_message.set_allocated_control_delegation_msg(control_delegation_msg);
  req_manager_.send_message(agent_id, d_message);
}

void flexran::app::scheduler::flexible_scheduler::enable_central_scheduling(bool central_sch) {
  central_scheduling.store(central_sch);

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());
  
  for (auto& agent_id : agent_ids) {

    std::string path = "";
    std::string remote_policy = "";
    std::string local_policy = "";
      
    if(const char* env_p = std::getenv("FLEXRAN_RTC_HOME")) {
      path = path + env_p + "/tests/delegation_control/";
    } else {
      path = "../tests/delegation_control/";
    }
    
    remote_policy = path + "remote_policy.yaml";
    local_policy = path + "local_policy.yaml";
    
    
    if (central_sch) {
      reconfigure_agent_file(agent_id, remote_policy);
    } else {
      reconfigure_agent_file(agent_id, local_policy);
    }
  }
}

void flexran::app::scheduler::flexible_scheduler::run_central_scheduler() {
  rib::frame_t target_frame;
  rib::subframe_t target_subframe;
  
  unsigned char aggregation = 1;
  //  uint16_t total_nb_available_rb[rib::MAX_NUM_CC];

  uint16_t nb_available_rb, nb_rb, nb_rb_tmp, TBS, sdu_length_total = 0;
  uint8_t harq_pid, round;

  uint32_t dci_tbs;
  int mcs = 0, tpc = 1, mcs_tmp;
  uint32_t ndi;
  uint32_t ce_flags = 0;
  uint32_t data_to_request;

  uint8_t header_len = 0, header_len_last = 0, ta_len = 0;
  
  bool ue_has_transmission = false;
  
  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());
  
  for (auto& agent_id : agent_ids) {
    
    protocol::flexran_message out_message;
    
    ::std::shared_ptr<rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
    const protocol::flex_enb_config_reply& enb_config = agent_config->get_enb_config();
    const protocol::flex_ue_config_reply& ue_configs = agent_config->get_ue_configs();
    const protocol::flex_lc_config_reply& lc_configs = agent_config->get_lc_configs();

    rib::frame_t current_frame = agent_config->get_current_frame();
    rib::subframe_t current_subframe = agent_config->get_current_subframe();

    // Check if scheduling context for this eNB is already present and if not create it
    ::std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(agent_id);
    if (enb_sched_info) {
      // Nothing to do if this exists
    } else { // eNB sched info was not found for this agent
      LOG4CXX_INFO(flog::app, "Config was not found. Creating");
      scheduling_info_.insert(::std::pair<int,
			      ::std::shared_ptr<enb_scheduling_info>>(agent_id,
								      ::std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
      enb_sched_info = get_scheduling_info(agent_id);
    }

    // Check if we have already run the scheduler for this particular time slot and if yes go to next eNB
    if (!needs_scheduling(enb_sched_info, current_frame, current_subframe)) {
      continue;
    }
    target_subframe = (current_subframe + schedule_ahead) % 10;
    if (target_subframe < current_subframe) {
      target_frame = (current_frame + 1) % 1024;
    } else {
      target_frame = current_frame;
    }
    int additional_frames = schedule_ahead / 10;
    target_frame = (target_frame + additional_frames) % 1024;

    ///if ((target_subframe != 1) && (target_subframe != 4) && (target_subframe != 6) && (target_subframe != 8)) {
    //  continue;
    //}
    
    if ((target_subframe  == 0) || (target_subframe == 5)) {
      continue;
    }
    LOG4CXX_DEBUG(flog::app, "Scheduling for frame " << target_frame << " and subframe " << target_subframe);

    // Create dl_mac_config message header
    protocol::flex_header *header(new protocol::flex_header);
    header->set_type(protocol::FLPT_DL_MAC_CONFIG);
    header->set_version(0);
    header->set_xid(0);

    // Create dl_mac_config message
    protocol::flex_dl_mac_config *dl_mac_config_msg(new protocol::flex_dl_mac_config);
    dl_mac_config_msg->set_allocated_header(header);
    dl_mac_config_msg->set_sfn_sf(rib::get_sfn_sf(target_frame, target_subframe));
    
    // Go through the cell configs and set the variables
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      protocol::flex_cell_config cell_config = enb_config.cell_config(i);
      //total_nb_available_rb[i] = cell_config.dl_bandwidth();
      //Remove the RBs used by common channels
      //TODO: For now we will do this manually based on OAI config and scheduling sf. Important to fix it later.
      // Assume an FDD scheduler
      // switch(target_subframe) {
      // case 0:
      // 	total_nb_available_rb[i] -= 4;
      // 	break;
      // case 5:
      // 	total_nb_available_rb[i] -= 8;
      // 	break;
      // }
      enb_sched_info->start_new_scheduling_round(target_subframe, cell_config);

      // Run the preprocessor to make initial allocation of RBs to UEs (Need to do this over all scheduling_info of eNB)
      remote_scheduler_helper::run_dlsch_scheduler_preprocessor(cell_config, ue_configs, lc_configs, agent_config, enb_sched_info, target_frame, target_subframe);
    }

    // Go through the cells and schedule the UEs of this cell
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      protocol::flex_cell_config cell_config = enb_config.cell_config(i);
      uint16_t cell_id = cell_config.cell_id();
      

      for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
	protocol::flex_ue_config ue_config = ue_configs.ue_config(UE_id);
	if (ue_config.pcell_carrier_index() == cell_id) {

	  // Get the MAC stats for this UE
	  ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	  
	  // Get the scheduling info
	  ::std::shared_ptr<ue_scheduling_info> ue_sched_info = enb_sched_info->get_ue_scheduling_info(ue_config.rnti());
	  // if (ue_sched_info) {
	  //   ue_sched_info->start_new_scheduling_round();
	  // } else { // we need to create the scheduling info
	  //   enb_sched_info->create_ue_scheduling_info(ue_config.rnti());
	  //   ue_sched_info = enb_sched_info->get_ue_scheduling_info(ue_config.rnti());
	  // }
	  // Check if we have stats for this UE
	  if (!ue_mac_info) {
	    continue;
	  }

	  protocol::flex_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();

	  for (int j = 0; j < mac_report.dl_cqi_report().csi_report_size(); j++) {
	    if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(j).serv_cell_index()) {
	      aggregation = get_aggregation(get_bw_index(cell_config.dl_bandwidth()),
					    mac_report.dl_cqi_report().csi_report(j).p10csi().wb_cqi(),
					    protocol::FLDCIF_1);
	      break;
	    }
	  }	 
	  
	  // Schedule this UE
	  // Check if the preprocessor allocated rbs for this and if
	  // CCE allocation is feasible
	  if (CCE_allocation_infeasible(enb_sched_info, cell_config, ue_config, aggregation, target_subframe)) {
	    LOG4CXX_DEBUG(flog::app, "CCE allocation was infeasible");
	    continue;
	  }

	  if (!ue_mac_info->has_available_harq(cell_id)) {
	    continue;
	  }
	  
	  
	  if ((ue_sched_info->get_pre_nb_rbs_available(cell_id) == 0)) {
	    continue;
	  }


	  nb_available_rb = ue_sched_info->get_pre_nb_rbs_available(cell_id);

	  harq_pid = ue_mac_info->get_next_available_harq(cell_id);
	  round = ue_mac_info->get_harq_stats(cell_id, harq_pid);//ue_sched_info->get_harq_round(cell_id, harq_pid);

	  sdu_length_total = 0;

	  for (int j = 0; j < mac_report.dl_cqi_report().csi_report_size(); j++) {
	    if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(j).serv_cell_index()) {
	      mcs = rib::cqi_to_mcs[mac_report.dl_cqi_report().csi_report(j).p10csi().wb_cqi()];
	      break;
	    }
	  }

	  // Create a dl_data message
	  protocol::flex_dl_data *dl_data = dl_mac_config_msg->add_dl_ue_data();

	  if (round > 0) {
	    
	    // Use the MCS that was previously assigned to this HARQ process
	    mcs = ue_sched_info->get_mcs(cell_id, harq_pid);
	    nb_rb = ue_sched_info->get_nb_rbs_required(cell_id);

	    dci_tbs = get_TBS_DL(mcs, nb_rb);

	    if (nb_rb <= nb_available_rb) {
	      if (nb_rb == nb_available_rb) {
		//Set the already allocated subband allocation
		for (int j = 0; j < get_nb_rbg(cell_config); j++) {
		  ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							   harq_pid,
							   j,
							   ue_sched_info->get_rballoc_sub(cell_id, j));
		}
	      } else {
		nb_rb_tmp = nb_rb;
		int j = 0;

		while ((nb_rb_tmp > 0) && (j < get_nb_rbg(cell_config))) {
		  if (ue_sched_info->get_rballoc_sub(cell_id, j) == 1) {
		    ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							     harq_pid,
							     j,
							     ue_sched_info->get_rballoc_sub(cell_id, j));
		    if ((j == get_nb_rbg(cell_config) - 1) &&
			((cell_config.dl_bandwidth() == 25) ||
			 (cell_config.dl_bandwidth() == 50))) {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config) + 1;
		    } else {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config);
		    }
		  }
		  j++;
		}
	      }

	      nb_available_rb -= nb_rb;


	      for (int j = 0; j < mac_report.dl_cqi_report().csi_report_size(); j++) {
		if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(j).serv_cell_index()) {
		  aggregation = get_aggregation(get_bw_index(cell_config.dl_bandwidth()),
						mac_report.dl_cqi_report().csi_report(j).p10csi().wb_cqi(),
						protocol::FLDCIF_1);
		  break;
		}
	      }
	      ndi = ue_sched_info->get_ndi(cell_id, harq_pid);
	      tpc = ue_sched_info->get_tpc(cell_id, harq_pid);
	      ue_has_transmission = true;
	    } else {
	      // Do not schedule. The retransmission takes more resources than what we have
	      ue_has_transmission = false;
	    }
	  } else { /* This is potentially a new SDU opportunity */	    
	    TBS = get_TBS_DL(mcs, nb_available_rb);
	    dci_tbs = TBS;
	    
	    if (ue_sched_info->get_ta_timer() == 0) {
	      if (mac_report.pending_mac_ces() & protocol::FLPCET_TA) {
 		ta_len = 2;
		// Check if we need to update
		ue_sched_info->set_ta_timer(20);
 	      } else {
 		ta_len = 0;
 	      }
	    } else {
	      ue_sched_info->decr_ta_timer();
	      ta_len = 0;
	    }

	    if (ta_len > 0) {
	      ce_flags |= protocol::FLPCET_TA;
	    }

	    header_len = 0; // 2 bytes DCCH SDU subheader
	    header_len_last = 0;
	    sdu_length_total = 0;
	    // TODO: Need to make this prioritized
	    // Loop through the UE logical channels
	    for (int j = 1; j < mac_report.rlc_report_size() + 1; j++) {
	      header_len += 3;
	      protocol::flex_rlc_bsr *rlc_report = mac_report.mutable_rlc_report(j-1);

	      if (dci_tbs - ta_len - header_len - sdu_length_total > 0) {
		if (rlc_report->tx_queue_size() > 0) {
		  data_to_request = ::std::min(dci_tbs - ta_len - header_len - sdu_length_total, rlc_report->tx_queue_size());
		  if (data_to_request < 128) { // The header will be one byte less
		    header_len--;
		    header_len_last = 2;
		  } else {
		    header_len_last = 3;
		  }
		  // if ((j == 1) || (j == 2)) {
		  //   data_to_request++; // It is not correct but fixes some RLC bug for DCCH
		  // }
		  
		  protocol::flex_rlc_pdu *rlc_pdu = dl_data->add_rlc_pdu();
		  protocol::flex_rlc_pdu_tb *tb1 = rlc_pdu->add_rlc_pdu_tb();
		  protocol::flex_rlc_pdu_tb *tb2 = rlc_pdu->add_rlc_pdu_tb();
		  tb1->set_logical_channel_id(rlc_report->lc_id());
		  tb2->set_logical_channel_id(rlc_report->lc_id());
		  tb1->set_size(data_to_request);
		  tb2->set_size(data_to_request);
		  rlc_report->set_tx_queue_size(rlc_report->tx_queue_size() - data_to_request);
		  //Set this to the max value that we might request
		  sdu_length_total += data_to_request;
		} else {
		  header_len -= 3;
		} //End tx_queue_size == 0
	      } // end of if dci_tbs - ta_len - header_len > 0
	      
	    } // End of iterating the logical channels

	    if (header_len == 0) {
	      header_len_last = 0;
	    }
	    
	    // There is a payload
	    if ( dl_data->rlc_pdu_size() > 0) {
	      // Now compute the number of required RBs for total sdu length
	      // Assume RAH format 2
	      //Adjust header lengths
	      //	      header_len_tmp = header_len;

	      if (header_len != 0) {
		header_len_last--;
		header_len -= header_len_last;
	      }
	      
	      mcs_tmp = mcs;

	      if (mcs_tmp == 0) {
		nb_rb = 4; // don't let the TBS get too small
	      } else {
		nb_rb = get_min_rb_unit(cell_config);
	      }

	      TBS = get_TBS_DL(mcs_tmp, nb_rb);

	      while (TBS < (sdu_length_total + header_len + ta_len)) {
		nb_rb += get_min_rb_unit(cell_config);
		if (nb_rb > nb_available_rb) { // If we've gone beyond the maximum number of RBs
		  TBS = get_TBS_DL(mcs_tmp, nb_available_rb);
		  nb_rb = nb_available_rb;
		  break;
		}
		TBS = get_TBS_DL(mcs_tmp, nb_rb);
	      }

	      if (nb_rb == ue_sched_info->get_pre_nb_rbs_available(cell_id)) {
		// We have the exact number of RBs required. Just fill the rballoc subband
		for (int j = 0; j < get_nb_rbg(cell_config); j++) {
		  ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							   harq_pid,
							   j,
							   ue_sched_info->get_rballoc_sub(cell_id, j));
		}
	      } else {
		nb_rb_tmp = nb_rb;
		int j = 0;
		while ((nb_rb_tmp > 0) && (j < get_nb_rbg(cell_config))) {
		  if (ue_sched_info->get_rballoc_sub(cell_id, j) == 1) {
		    ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							     harq_pid,
							     j,
							     1);
		    if ((j == get_nb_rbg(cell_config) - 1) &&
			((cell_config.dl_bandwidth() == 25) ||
			 (cell_config.dl_bandwidth() == 50))) {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config) + 1;
		    } else {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config);
		    }
		  }
		  j++;
		}
	      }

	      // decrease MCS until TBS falls below required length
	      while ((TBS > (sdu_length_total + header_len + ta_len)) && (mcs_tmp > 0)) {
		mcs_tmp--;
		TBS = get_TBS_DL(mcs_tmp, nb_rb);
	      }

	      // If we have decreased too much we don't have enough RBs, increase MCs
	      while ((TBS < (sdu_length_total + header_len + ta_len)) && (mcs_tmp < 28)) {
		     // (((ue_sched_info->get_dl_power_offset(cell_id) > 0) && (mcs_tmp < 28)) ||
		     // ((ue_sched_info->get_dl_power_offset(cell_id) == 0) && (mcs_tmp <= 15)))) {
		mcs_tmp++;
		TBS = get_TBS_DL(mcs_tmp, nb_rb);
	      }

	      dci_tbs = TBS;
	      mcs = mcs_tmp;
	      
	      LOG4CXX_DEBUG(flog::app, "Decided MCS, nb_rb and TBS are " << mcs << " " << nb_rb << " " << dci_tbs);
	      // Update the mcs used for this harq process
	      ue_sched_info->set_mcs(cell_id, harq_pid, mcs);

	      ue_sched_info->set_nb_scheduled_rbs(cell_id, harq_pid, nb_rb);

	      // do PUCCH power control
	      // This is the normalized RX power
	      rib::cell_mac_rib_info& cell_rib_info = agent_config->get_cell_mac_rib_info(cell_id);
	      protocol::flex_cell_stats_report& cell_report = cell_rib_info.get_cell_stats_report();

	      int16_t normalized_rx_power;
	      bool rx_power_needs_update = false;
	      
	      for (int k = 0; k < mac_report.ul_cqi_report().pucch_dbm_size(); k++) {
		if (mac_report.ul_cqi_report().pucch_dbm(k).serv_cell_index() == cell_id) {
		  if (mac_report.ul_cqi_report().pucch_dbm(k).has_p0_pucch_dbm()) {
		    normalized_rx_power = mac_report.ul_cqi_report().pucch_dbm(k).p0_pucch_dbm();
		    if (mac_report.ul_cqi_report().pucch_dbm(k).p0_pucch_updated() == 1) {
		      rx_power_needs_update = true;
		    }
		    break;
		  }
		}
	      }
	      
	      int16_t target_rx_power = cell_report.noise_inter_report().p0_nominal_pucch() + 20;
	      
	      int32_t framex10psubframe = ue_sched_info->get_pucch_tpc_tx_frame()*10 + ue_sched_info->get_pucch_tpc_tx_subframe();

	      if (((framex10psubframe+10) <= (target_frame*10 + target_subframe)) || // normal case
		  ((framex10psubframe > (target_frame*10 + target_subframe)) && (((10240 - framex10psubframe + target_frame*10+target_subframe) >= 10 )))) {// Frame wrap-around
		if (rx_power_needs_update) {
		  ue_sched_info->set_pucch_tpc_tx_frame(target_frame);
		  ue_sched_info->set_pucch_tpc_tx_subframe(target_subframe);
		  if (normalized_rx_power > (target_rx_power+1)) {
		    tpc = 0; //-1
		    tpc_accumulated--;
		  } else if (normalized_rx_power < (target_rx_power - 1)) {
		    tpc = 2; //+1
		    tpc_accumulated++;
		  } else {
		    tpc = 1; //0
		  }
		} else {// Po_PUCCH has been updated
		  tpc = 1;
		} // time to do TPC update
	      } else {
		tpc = 1; //0
	      }
	      ue_sched_info->toggle_ndi(cell_id, harq_pid);
	      ndi = ue_sched_info->get_ndi(cell_id, harq_pid);
	      ue_sched_info->set_tpc(cell_id, harq_pid, tpc);
	      ue_has_transmission = true;
	    } else { // There is no data to transmit, so don't schedule
	      ue_has_transmission = false;
	    }
	  }
	  
	  // If we had a new transmission or retransmission
	  if (ue_has_transmission) {
	    // After this point all UEs will be scheduled
	    dl_data->set_rnti(ue_config.rnti());
	    dl_data->set_serv_cell_index(cell_id);

	    // Add the control element flags to the flexran message
	    dl_data->add_ce_bitmap(ce_flags);
	    dl_data->add_ce_bitmap(ce_flags);

	    protocol::flex_dl_dci *dl_dci(new protocol::flex_dl_dci);
	    dl_data->set_allocated_dl_dci(dl_dci);
	    
	    dl_dci->set_rnti(ue_config.rnti());
	    dl_dci->set_harq_process(harq_pid);
	    ue_mac_info->harq_scheduled(cell_id, harq_pid);

	    // TODO: Currently set to static value. Need to create a function to obtain this
	    //	    aggregation = 1;
	    dl_dci->set_aggr_level(aggregation);
	    
	    enb_sched_info->assign_CCE(cell_id, 1<<aggregation);
	    
	    switch(ue_config.transmission_mode()) {
	    case 1:
	    case 2:
	    default:
	      dl_dci->set_res_alloc(0);
	      dl_dci->set_vrb_format(protocol::FLVRBF_LOCALIZED);
	      dl_dci->set_format(protocol::FLDCIF_1);
	      dl_dci->set_rb_shift(0);
	      dl_dci->add_ndi(ndi);
	      dl_dci->add_rv((round % 4));
	      dl_dci->set_tpc(tpc);
	      dl_dci->add_mcs(mcs);
	      dl_dci->add_tbs_size(dci_tbs);
	      dl_dci->set_rb_bitmap(allocate_prbs_sub(nb_rb,
						      ue_sched_info->get_rballoc_sub_scheduled(cell_id, harq_pid),
						      cell_config));
	    }
	  } else {
	    dl_mac_config_msg->mutable_dl_ue_data()->RemoveLast();
	  }
	}
      }
    }
    // Done with scheduling of eNB UEs. Set the last scheduled frame and subframe
    enb_sched_info->set_last_checked_frame(current_frame);
    enb_sched_info->set_last_checked_subframe(current_subframe);

    // Create and send the flexran message
    out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
    out_message.set_allocated_dl_mac_config_msg(dl_mac_config_msg);
    if (dl_mac_config_msg->dl_ue_data_size() > 0) {
      LOG4CXX_DEBUG(flog::app, "Scheduled " << dl_mac_config_msg->dl_ue_data_size() << " UEs in this round\n");
      req_manager_.send_message(agent_id, out_message);
    }
  }
}

std::shared_ptr<flexran::app::scheduler::enb_scheduling_info>
flexran::app::scheduler::flexible_scheduler::get_scheduling_info(int agent_id) {
  auto it = scheduling_info_.find(agent_id);
  if (it != scheduling_info_.end()) {
    return it->second;
  }
  return ::std::shared_ptr<enb_scheduling_info>(nullptr);
}

void flexran::app::scheduler::flexible_scheduler::push_cell_config_reconfiguration(
    int agent_id, const protocol::flex_cell_config& cell_config)
{
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);

  protocol::flex_enb_config_reply *enb_config_msg(new protocol::flex_enb_config_reply);
  enb_config_msg->add_cell_config();
  enb_config_msg->mutable_cell_config(0)->CopyFrom(cell_config);
  enb_config_msg->set_allocated_header(config_header);

  protocol::flexran_message config_message;
  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_enb_config_reply_msg(enb_config_msg);
  req_manager_.send_message(agent_id, config_message);
}

void flexran::app::scheduler::flexible_scheduler::push_ue_config_reconfiguration(
    int agent_id, const protocol::flex_ue_config_reply& ue_config)
{
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);

  protocol::flex_ue_config_reply *ue_config_msg(new protocol::flex_ue_config_reply);
  ue_config_msg->CopyFrom(ue_config);
  ue_config_msg->set_allocated_header(config_header);

  protocol::flexran_message config_message;
  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_ue_config_reply_msg(ue_config_msg);
  req_manager_.send_message(agent_id, config_message);
}

bool flexran::app::scheduler::flexible_scheduler::verify_dl_slice_config(
    const protocol::flex_dl_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [0,255]";
    return false;
  }
  /* label is enum */
  if (s.has_percentage() && (s.percentage() < 1 || s.percentage() > 100)) {
    error_message = "DL percentage must be within [1,100]";
    return false;
  }
  /* isolation can only be true or false */
  if (s.has_priority() && s.priority() > 20) {
    error_message = "priority must be within [0,20]";
    return false;
  }
  if (s.has_position_low() && s.position_low() > 25) {
    error_message = "position_low must be within [0,25] (RBG)";
    return false;
  }
  if (s.has_position_high() && s.position_high() > 25) {
    error_message = "position_high must be within [0,25] (RBG)";
    return false;
  }
  if (s.has_position_low() && s.has_position_high()
      && s.position_low() >= s.position_high()) {
    error_message = "position_low must be smaller than position_high";
    return false;
  }
  if (s.has_maxmcs() && s.maxmcs() > 28) {
    error_message = "DL maxmcs must be within [0,28]";
    return false;
  }
  /* sorting is enum */
  /* accounting is enum */
  if (s.has_scheduler_name()) {
    error_message = "setting another scheduler is not supported";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_dl_slice_removal(
    const protocol::flex_dl_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [1,255]";
    return false;
  }
  if (s.id() == 0) {
    error_message = "DL Slice 0 can not be deleted";
    return false;
  }
  if (!s.has_percentage() || s.percentage() != 0) {
    error_message = "Slice removal requires percentage to be set to 0";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_ul_slice_config(
    const protocol::flex_ul_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [0,255]";
    return false;
  }
  /* label is enum */
  if (s.has_percentage() && (s.percentage() < 1 || s.percentage() > 100)) {
    error_message = "percentage must be within [1,100]";
    return false;
  }
  /* isolation can only be true or false */
  if (s.has_priority()) {
    error_message = "slice priority is not supported";
    return false;
  }
  if (s.has_first_rb() && s.first_rb() > 99) {
    error_message = "first_rb must be within [0,99] (RB)";
    return false;
  }
  /*if (s.has_length_rb()
      && (s.length_rb() < 1 || s.length_rb() > 100)) {
    error_message = "length_rb must be within [1,100] (RB)";
    return false;
  }
  if (s.has_length_rb() && s.has_first_rb() && s.length_rb() + s.first_rb() > 100) {
    error_message = "length_rb must be within [1,100-first_rb] (RB)";
    return false;
  }*/
  if (s.has_maxmcs() && s.maxmcs() > 20) {
    error_message = "UL maxmcs must be within [0,20]";
    return false;
  }
  /* sorting is enum */
  /* accounting is enum */
  if (s.has_scheduler_name()) {
    error_message = "setting another scheduler is not supported";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_ul_slice_removal(
    const protocol::flex_ul_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [1,255]";
    return false;
  }
  if (s.id() == 0) {
    error_message = "UL Slice 0 can not be deleted";
    return false;
  }
  if (!s.has_percentage() || s.percentage() != 0) {
    error_message = "Slice removal requires percentage to be set to 0";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_global_slice_percentage(
    int agent_id, const protocol::flex_slice_config& c, std::string& error_message)
{
  auto h = rib_.get_agent(agent_id);
  if (h == nullptr) {
    error_message = "no such agent";
    return false;
  }
  const protocol::flex_slice_config& ex = h->get_enb_config().cell_config(0).slice_config();
  return verify_global_dl_slice_percentage(ex, c, error_message)
      && verify_global_ul_slice_percentage(ex, c, error_message);
}

bool flexran::app::scheduler::flexible_scheduler::verify_global_dl_slice_percentage(
    const protocol::flex_slice_config& existing,
    const protocol::flex_slice_config& update, std::string& error_message)
{
  std::map<int, int> slice_pct;
  for (int i = 0; i < existing.dl_size(); i++)
    slice_pct[existing.dl(i).id()] = existing.dl(i).percentage();
  for (int i = 0; i < update.dl_size(); i++)
    // the agent will copy the values from slice 0 if not specified, so do we
    slice_pct[update.dl(i).id()] = update.dl(i).has_percentage() ?
        update.dl(i).percentage() : slice_pct[0];
  int sum = 0;
  for (const auto &p: slice_pct)
    sum += p.second;
  if (sum > 100) {
    error_message = "resulting DL slice sum percentage exceeds 100";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_global_ul_slice_percentage(
    const protocol::flex_slice_config& existing,
    const protocol::flex_slice_config& update, std::string& error_message)
{
  std::map<int, int> slice_pct;
  for (int i = 0; i < existing.ul_size(); i++)
    slice_pct[existing.ul(i).id()] = existing.ul(i).percentage();
  for (int i = 0; i < update.ul_size(); i++)
    // the agent will copy the values from slice 0 if not specified, so do we
    slice_pct[update.ul(i).id()] = update.ul(i).has_percentage() ?
        update.ul(i).percentage() : slice_pct[0];
  int sum = 0;
  for (const auto &p: slice_pct)
    sum += p.second;
  if (sum > 100) {
    error_message = "resulting UL slice sum percentage exceeds 100";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_ue_slice_assoc_msg(
    const protocol::flex_ue_config& c, std::string& error_message)
{

  if (!c.has_rnti() && !c.has_imsi()) {
    error_message = "Missing RNTI or IMSI";
    return false;
  }
  if (!c.has_dl_slice_id() && !c.has_ul_slice_id()) {
    error_message = "No DL or UL slice ID";
    return false;
  }
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_rnti_imsi(
    int agent_id, protocol::flex_ue_config *c, std::string& error_message)
{
  // if RNTI present but there is no corresponding UE, abort
  if (c->has_rnti() && !rib_.get_agent(agent_id)->get_ue_mac_info(c->rnti())) {
    error_message = "a UE with RNTI" + std::to_string(c->rnti()) + " does not exist";
    return false;
  }

  // there is an RNTI, the corresponding UE exists and no IMSI that could
  // contradict -> can leave
  if (!c->has_imsi())
    return true;

  uint64_t imsi = c->imsi();
  flexran::rib::rnti_t rnti;
  if (!rib_.get_agent(agent_id)->get_rnti(imsi, rnti)) {
    error_message = "IMSI " + std::to_string(imsi) + " is not present";
    return false;
  }

  if (rnti == 0) {
      error_message = "found invalid RNTI 0 for IMSI " + std::to_string(imsi);
      return false;
  }

  if (c->has_rnti() && c->rnti() != rnti) {
    error_message = "RNTI-IMSI mismatch";
    return false;
  }

  c->set_rnti(rnti);
  return true;
}

bool flexran::app::scheduler::flexible_scheduler::verify_cell_config_for_restart(
    const protocol::flex_cell_config& c, std::string& error_message)
{
  if (c.has_phy_cell_id()) {
    error_message = "setting phy_cell_id not supported";
    return false;
  }
  if (c.has_cell_id()) {
    error_message = "setting cell_id not supported";
    return false;
  }
  if (c.has_pusch_hopping_offset()) {
    error_message = "setting pusch_hopping_offset not supported";
    return false;
  }
  if (c.has_hopping_mode()) {
    error_message = "setting hopping_mode not supported";
    return false;
  }
  if (c.has_n_sb()) {
    error_message = "setting n_sb not supported";
    return false;
  }
  if (c.has_phich_resource()) {
    error_message = "setting phich_resource not supported";
    return false;
  }
  if (c.has_phich_duration()) {
    error_message = "setting phich_durationnot supported";
    return false;
  }
  if (c.has_init_nr_pdcch_ofdm_sym()) {
    error_message = "setting init_nr_pdcch_ofdm_sym not supported";
    return false;
  }
  if (c.has_si_config()) {
    error_message = "setting si_config not supported";
    return false;
  }
  if (c.has_ul_cyclic_prefix_length()) {
    error_message = "setting ul_cyclic_prefix_length not supported";
    return false;
  }
  if (c.has_dl_cyclic_prefix_length()) {
    error_message = "setting dl_cyclic_prefix_length not supported";
    return false;
  }
  if (c.has_antenna_ports_count()) {
    error_message = "setting antenna_ports_count not supported";
    return false;
  }
  if (c.has_duplex_mode()) {
    error_message = "setting duplex_mode not supported";
    return false;
  }
  if (c.has_subframe_assignment()) {
    error_message = "setting subframe_assignment not supported";
    return false;
  }
  if (c.has_special_subframe_patterns()) {
    error_message = "setting special_subframe_patterns not supported";
    return false;
  }
  if (c.mbsfn_subframe_config_rfperiod_size() > 0) {
    error_message = "setting mbsfn_subframe_config_rfperiod not supported";
    return false;
  }
  if (c.mbsfn_subframe_config_rfoffset_size() > 0) {
    error_message = "setting mbsfn_subframe_config_rfoffset not supported";
    return false;
  }
  if (c.mbsfn_subframe_config_sfalloc_size() > 0) {
    error_message = "setting mbsfn_subframe_config_sfalloc not supported";
    return false;
  }
  if (c.has_prach_config_index()) {
    error_message = "setting prach_config_index not supported";
    return false;
  }
  if (c.has_prach_freq_offset()) {
    error_message = "setting prach_freq_offset not supported";
    return false;
  }
  if (c.has_ra_response_window_size()) {
    error_message = "setting ra_response_window_size not supported";
    return false;
  }
  if (c.has_mac_contention_resolution_timer()) {
    error_message = "setting mac_contention_resolution_timer not supported";
    return false;
  }
  if (c.has_max_harq_msg3tx()) {
    error_message = "setting max_harq_msg3tx not supported";
    return false;
  }
  if (c.has_n1pucch_an()) {
    error_message = "setting n1pucch_an not supported";
    return false;
  }
  if (c.has_deltapucch_shift()) {
    error_message = "setting deltapucch_shift not supported";
    return false;
  }
  if (c.has_nrb_cqi()) {
    error_message = "setting nrb_cqi not supported";
    return false;
  }
  if (c.has_srs_subframe_config()) {
    error_message = "setting srs_subframe_config not supported";
    return false;
  }
  if (c.has_srs_bw_config()) {
    error_message = "setting srs_bw_config not supported";
    return false;
  }
  if (c.has_srs_mac_up_pts()) {
    error_message = "setting srs_mac_up_pts not supported";
    return false;
  }
  if (c.has_enable_64qam()) {
    error_message = "setting enable_64qam not supported";
    return false;
  }
  if (c.has_carrier_index()) {
    error_message = "setting not supported yet, defaults to 0";
    return false;
  }
  if (c.has_slice_config()) {
    error_message = "setting slice_config not supported, use another end point";
    return false;
  }
  /* if no band is given, we simply assume band 7 */
  if (!c.has_eutra_band()) {
    error_message = "eutra_band must be present";
    return false;
  }
  if (!c.has_dl_freq() || !c.has_ul_freq()) {
    error_message = "both dl_freq and ul_freq must be present";
    return false;
  }
  if (!c.has_dl_bandwidth() || !c.has_ul_bandwidth()) {
    error_message = "both dl_bandwidth and ul_bandwidth must be present";
    return false;
  }
  if (c.dl_bandwidth() != c.ul_bandwidth()) {
    error_message = "dl_bandwidth and ul_bandwidth must be the same (6, 15, 25, 50, 100)";
    return false;
  }
  if (!check_eutra_bandwidth(c.dl_bandwidth(), error_message))
    return false;
  // checking function tests against Hz, but ul_freq/dl_freq are in MHz!
  if (!check_eutra_band(c.eutra_band(), c.ul_freq() * 1000000, c.dl_freq() * 1000000, error_message, c.dl_bandwidth(), true))
    return false;

  return true;
}

int flexran::app::scheduler::flexible_scheduler::parse_enb_agent_id(
    const std::string& enb_agent_id_s) const
{
  return rib_.parse_enb_agent_id(enb_agent_id_s);
}

int flexran::app::scheduler::flexible_scheduler::get_last_agent() const
{
  if (rib_.get_available_agents().empty())
    return -1;

  return *std::prev(rib_.get_available_agents().end());
}

bool flexran::app::scheduler::flexible_scheduler::parse_rnti_imsi(
    int agent_id, const std::string& rnti_imsi_s,
    flexran::rib::rnti_t& rnti) const
{
  return rib_.get_agent(agent_id)->parse_rnti_imsi(rnti_imsi_s, rnti);
}

bool flexran::app::scheduler::flexible_scheduler::try_add_first_rb(
    int agent_id, protocol::flex_ul_slice& slice)
{
  // this function is dumb: it simply assumes that all existing slices are
  // adjacent and only one is added. Therefore, it picks the highest first_Rb,
  // adds N_RB * percentage and adds this to the existing slice.
  auto h = rib_.get_agent(agent_id);
  if (h == nullptr) return false;
  const protocol::flex_slice_config& ex = h->get_enb_config().cell_config(0).slice_config();
  if (ex.ul_size() < 1) return false;
  const int N_RB = h->get_enb_config().cell_config(0).ul_bandwidth();
  const int pct = ex.ul(0).percentage();
  const int first_rb = ex.ul(ex.ul_size() - 1).first_rb();
  slice.set_first_rb(first_rb + pct * N_RB / 100);
  return true;
}

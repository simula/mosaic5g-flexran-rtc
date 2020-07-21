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

/*! \file    rib_management.cc
 *  \brief   app polling agents for connection management purposes
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include "rt_controller_common.h"
#include "rib_management.h"
#include "enb_rib_info.h"
#include "flexran_log.h"

flexran::app::management::rib_management::rib_management(const flexran::rib::Rib& rib,
    const flexran::core::requests_manager& rm, flexran::event::subscription& sub)
  : component(rib, rm, sub)
{
  event_sub_.subscribe_task_tick(
      boost::bind(&flexran::app::management::rib_management::tick, this, _1), 1000);
}

void flexran::app::management::rib_management::tick(uint64_t ms)
{
  _unused(ms);
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  for (uint64_t bs_id: rib_.get_available_base_stations()) {   
    send_enb_config_request(bs_id);
    send_ue_config_request(bs_id);
    send_lc_config_request(bs_id);
    std::chrono::duration<float> inactive = now - rib_.get_bs(bs_id)->last_active();
    /* inactive for longer than 1.5s */
    if (inactive.count() >= 1.5) {
      inactive_bs_.insert(bs_id);
      LOG4CXX_WARN(flog::app, "RibManagement: no connection to BS " << bs_id
          << " since " << inactive.count() << "s");
    } else {
      if (inactive_bs_.find(bs_id) != inactive_bs_.end()) {
        inactive_bs_.erase(bs_id);
        LOG4CXX_INFO(flog::app, "RibManagement: connection to BS " << bs_id
            << " is now active again");
      }
    }
  }
}

void flexran::app::management::rib_management::send_enb_config_request(uint64_t bs_id)
{
  // request eNB config file
  protocol::flex_header *header1(new protocol::flex_header);
  header1->set_type(protocol::FLPT_GET_ENB_CONFIG_REQUEST);
  header1->set_version(0);
  header1->set_xid(0);

  protocol::flex_enb_config_request *enb_config_request_msg(new protocol::flex_enb_config_request);
  enb_config_request_msg->set_allocated_header(header1);

  protocol::flexran_message out_message1;
  out_message1.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message1.set_allocated_enb_config_request_msg(enb_config_request_msg);
  req_manager_.send_message(bs_id, out_message1);
}

void flexran::app::management::rib_management::send_ue_config_request(uint64_t bs_id)
{
  // request eNB config file
  protocol::flex_header *header1(new protocol::flex_header);
  header1->set_type(protocol::FLPT_GET_UE_CONFIG_REQUEST);
  header1->set_version(0);
  header1->set_xid(0);

  protocol::flex_ue_config_request *ue_config_request_msg(new protocol::flex_ue_config_request);
  ue_config_request_msg->set_allocated_header(header1);

  protocol::flexran_message out_message1;
  out_message1.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message1.set_allocated_ue_config_request_msg(ue_config_request_msg);
  req_manager_.send_message(bs_id, out_message1);
}

void flexran::app::management::rib_management::send_lc_config_request(uint64_t bs_id)
{
  protocol::flex_header *header1(new protocol::flex_header);
  header1->set_type(protocol::FLPT_GET_LC_CONFIG_REQUEST);
  header1->set_version(0);
  header1->set_xid(0);

  protocol::flex_lc_config_request *lc_config_request_msg(new protocol::flex_lc_config_request);
  lc_config_request_msg->set_allocated_header(header1);

  protocol::flexran_message out_message1;
  out_message1.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message1.set_allocated_lc_config_request_msg(lc_config_request_msg);
  req_manager_.send_message(bs_id, out_message1);
}

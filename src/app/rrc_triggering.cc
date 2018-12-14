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

/*! \file    rrc_triggering.cc
 *  \brief   trigger RRC measurements at the agent
 *  \authors Shahab SHARIAT BAGHERI
 *  \company Eurecom
 *  \email   shahab.shariat@eurecom.fr
 */

#include <iostream>
#include <string>
#include <streambuf>
#include <fstream>


#include "rrc_triggering.h"
#include "flexran.pb.h"
#include "rib_common.h"


void flexran::app::rrc::rrc_triggering::periodic_task() {

  //::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

  /*TODO for Handover*/
}


void flexran::app::rrc::rrc_triggering::reconfigure_agent(uint64_t bs_id, std::string freq_measure) {
  
  protocol::flexran_message config_message;
  // Create control delegation message header
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_RRC_TRIGGERING);
  header->set_version(0);
  header->set_xid(0);
  
  protocol::flex_rrc_triggering *agent_rrc_triggering(new protocol::flex_rrc_triggering);
  agent_rrc_triggering->set_allocated_header(header);

  agent_rrc_triggering->set_rrc_trigger(freq_measure);

  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_rrc_triggering(agent_rrc_triggering);
  req_manager_.send_message(bs_id, config_message);
}


void flexran::app::rrc::rrc_triggering::enable_rrc_triggering(std::string freq_measure) {
  

  for (uint64_t bs_id : rib_.get_available_base_stations()) {

      reconfigure_agent(bs_id, freq_measure);
  
  }
}

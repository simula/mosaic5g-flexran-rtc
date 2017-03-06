/* The MIT License (MIT)

   Copyright (c) 2017

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

#include <iostream>
#include <string>
#include <streambuf>
#include <fstream>


#include "rrc_triggering.h"
#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"
#include "flexran.pb.h"
#include "rib_common.h"
#include "cell_mac_rib_info.h"



void flexran::app::rrc::rrc_triggering::reconfigure_agent(int agent_id) {
  // std::ifstream policy_file(policy_name);
  // std::string str_policy;

  // policy_file.seekg(0, std::ios::end);
  // str_policy.reserve(policy_file.tellg());
  // policy_file.seekg(0, std::ios::beg);

  // str_policy.assign((std::istreambuf_iterator<char>(policy_file)),
		//     std::istreambuf_iterator<char>());

  protocol::flexran_message config_message;
  // Create control delegation message header
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);
  
  protocol::flex_agent_reconfiguration *agent_reconfiguration_msg(new protocol::flex_agent_reconfiguration);
  agent_reconfiguration_msg->set_allocated_header(config_header);

  // agent_reconfiguration_msg->set_policy(str_policy);

  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_agent_reconfiguration_msg(agent_reconfiguration_msg);
  req_manager_.send_message(agent_id, config_message);
}


void flexran::app::rrc::rrc_triggering::enable_central_scheduling() {
  // central_scheduling.store(central_sch);

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());
  
  for (auto& agent_id : agent_ids) {

  //   if (central_sch) {
      reconfigure_agent(agent_id);
  //   } else {
  //     reconfigure_agent(agent_id, "../tests/delegation_control/local_policy.yaml");
  //   }
  }
}
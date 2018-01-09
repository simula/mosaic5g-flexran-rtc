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
#include "flexran.pb.h"
#include "rib_common.h"


void flexran::app::rrc::rrc_triggering::periodic_task() {

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

  /*TODO for Handover*/
}


void flexran::app::rrc::rrc_triggering::reconfigure_agent(int agent_id, std::string freq_measure) {
  
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
  req_manager_.send_message(agent_id, config_message);
}


void flexran::app::rrc::rrc_triggering::enable_rrc_triggering(std::string freq_measure) {
  

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());
  
  for (auto& agent_id : agent_ids) {

      reconfigure_agent(agent_id, freq_measure);
  
  }
}

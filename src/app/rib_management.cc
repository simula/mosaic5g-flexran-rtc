/* The MIT License (MIT)

   Copyright (c) 2017 Robert Schmidt

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

#include "rib_management.h"
#include "enb_rib_info.h"
#include "flexran_log.h"

void flexran::app::management::rib_management::run_periodic_task()
{
  // only execute every second
  ms_counter++;
  ms_counter %= 1000;
  if (ms_counter != 0) return;

  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  for (int agent_id: rib_.get_available_agents()) {
    send_enb_config_request(agent_id);
    std::chrono::duration<float> inactive = now - rib_.get_agent(agent_id)->last_active();
    /* inactive for longer than 1.5s */
    if (inactive.count() >= 1.5) {
      inactive_agents.insert(agent_id);
      LOG4CXX_WARN(flog::app, "RibManagement: no connection to agent " << agent_id
          << " since " << inactive.count() << "s");
    } else {
      if (inactive_agents.find(agent_id) != inactive_agents.end()) {
        inactive_agents.erase(agent_id);
        LOG4CXX_INFO(flog::app, "RibManagement: connection to agent " << agent_id
            << " is now active again");
      }
    }
  }
  last_now = now;
}

void flexran::app::management::rib_management::send_enb_config_request(int agent_id)
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
  req_manager_.send_message(agent_id, out_message1);
}

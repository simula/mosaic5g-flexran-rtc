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

/*! \file    requests_manager.cc
 *  \brief   bridge between the apps and the network interface
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#include "requests_manager.h"
#include "async_xface.h"
#include "rib.h"
#include "agent_info.h"
#include "flexran_log.h"

void flexran::core::requests_manager::send_message(uint64_t bs_id,
    const protocol::flexran_message& msg) const
{
  auto bs = rib_.get_bs(bs_id);
  if (!bs) {
    LOG4CXX_ERROR(flog::core, "RequestsManager: unknown BS ID " << bs_id);
    return;
  }
  /* TODO verify which agent really needs to receive this */
  for (auto a : bs->get_agents())
    net_xface_.send_msg(msg, a->agent_id);
}

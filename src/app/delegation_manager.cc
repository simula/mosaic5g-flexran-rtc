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

/*! \file    delegation_manager.cc
 *  \brief   example app for control delegation
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include <string>
#include <fstream>
#include <streambuf>

#include "delegation_manager.h"

bool flexran::app::management::delegation_manager::push_object(
    const std::string &bs, const std::string& name,
    const char *data, int len, std::string& error_reason)
{
  uint64_t bs_id = rib_.parse_enb_agent_id(bs);
  if (bs_id == 0) {
    error_reason = "can not find BS";
    return false;
  }

  if (name.empty()) {
    error_reason = "empty name";
    return false;
  }

  if (data == nullptr) {
    error_reason = "no data";
    return false;
  }

  if (len < 1 || len > 230000) {
    error_reason = "invalid object size " + std::to_string(len)
                    + "B, must be within [1,230000]B";
    return false;
  }

  push_code(bs_id, name, data, len);
  LOG4CXX_INFO(flog::app, "delegation: sent new object (name " << name
               << ", length " << len << ") to BS " << bs_id);
  return true;
}

void flexran::app::management::delegation_manager::push_code(
    uint64_t bs_id, const std::string& name, const char *data, int len)
{
  protocol::flexran_message msg;
  protocol::flex_header *delegation_header(new protocol::flex_header);
  delegation_header->set_type(protocol::FLPT_DELEGATE_CONTROL);
  delegation_header->set_version(0);
  delegation_header->set_xid(0);
  
  protocol::flex_control_delegation *delg(new protocol::flex_control_delegation);
  delg->set_allocated_header(delegation_header);
  delg->set_payload(data, len);
  delg->set_name(name);

  msg.set_msg_dir(protocol::INITIATING_MESSAGE);
  msg.set_allocated_control_delegation_msg(delg);
  req_manager_.send_message(bs_id, msg);
}

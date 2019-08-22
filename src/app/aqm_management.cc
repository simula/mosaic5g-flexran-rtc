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

/*! \file    aqm_management.cc
 *  \brief   handle Active Queue Management
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include "aqm_management.h"
#include "flexran_log.h"
#include "flexran.pb.h"

#include <google/protobuf/util/json_util.h>
namespace proto_util = google::protobuf::util;

flexran::app::management::aqm_management::aqm_management(const flexran::rib::Rib& rib,
    const flexran::core::requests_manager& rm, flexran::event::subscription& sub)
  : component(rib, rm, sub)
{
}

bool flexran::app::management::aqm_management::handle_aqm_config(
    const std::string& bs, const std::string& policy, std::string& error_reason)
{
  uint64_t bs_id = rib_.parse_enb_agent_id(bs);
  if (bs_id == 0) {
    error_reason = "can not find BS";
    return false;
  }

  protocol::flex_lc_config_reply lc_config;
  auto ret = proto_util::JsonStringToMessage(policy, &lc_config, proto_util::JsonParseOptions());
  if (ret != proto_util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf message:" << ret.ToString());
    return false;
  }

  std::shared_ptr<flexran::rib::enb_rib_info> bsp = rib_.get_bs(bs_id);
  for (const auto& c : lc_config.lc_ue_config()) {
    if (!c.has_rnti()) {
      error_reason = "no RNTI in lc_ue_config";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    const rib::rnti_t rnti = c.rnti();
    const auto ue_it = std::find_if(
        bsp->get_ue_configs().ue_config().begin(),
        bsp->get_ue_configs().ue_config().end(),
        [rnti] (const protocol::flex_ue_config& c) { return c.rnti() == rnti; }
    );
    if (ue_it == bsp->get_ue_configs().ue_config().end()) {
      error_reason = "no RNTI " + std::to_string(rnti) + " in BS " + std::to_string(bs_id);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (c.lc_config_size() > 0) {
      error_reason = "illegal lc_config detected for RNTI " + std::to_string(rnti);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (c.sdap_size() == 0) {
      error_reason = "no SDAP config present for RNTI " + std::to_string(rnti);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  send_lc_config_update(bs_id, lc_config);
  std::string s;
  proto_util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  proto_util::MessageToJsonString(lc_config, &s, opt);
  LOG4CXX_INFO(flog::app, "sent new AQM/SDAP configuration to BS " << bs_id << ":\n" << s);

  return true;
}

bool flexran::app::management::aqm_management::handle_aqm_config_rnti(
    const std::string& bs, const std::string& ue, const std::string& policy, std::string& error_reason)
{
  uint64_t bs_id = rib_.parse_enb_agent_id(bs);
  if (bs_id == 0) {
    error_reason = "can not find BS";
    return false;
  }

  rib::rnti_t rnti;
  if (ue != "-1") {
    if (!rib_.get_bs(bs_id)->parse_rnti_imsi(ue, rnti)) {
      error_reason = "can not find UE";
      return false;
    }
  } else {
    const protocol::flex_ue_config_reply& ues = rib_.get_bs(bs_id)->get_ue_configs();
    if (ues.ue_config().size() == 0) {
      error_reason = "can not find UE: no UE connected";
      return false;
    }
    rnti = (std::prev(ues.ue_config().end()))->rnti();
  }

  protocol::flex_sdap_config sdap_config;
  auto ret = proto_util::JsonStringToMessage(policy, &sdap_config, proto_util::JsonParseOptions());
  if (ret != proto_util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf message: " << ret.ToString());
    return false;
  }

  protocol::flex_lc_config_reply lc_config;
  lc_config.add_lc_ue_config()->set_rnti(rnti);
  lc_config.mutable_lc_ue_config(0)->add_sdap()->CopyFrom(sdap_config);
  send_lc_config_update(bs_id, lc_config);
  std::string s;
  proto_util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  proto_util::MessageToJsonString(lc_config, &s, opt);
  LOG4CXX_INFO(flog::app, "sent new AQM/SDAP configuration to BS " << bs_id << ":\n" << s);

  return true;
}

void flexran::app::management::aqm_management::send_lc_config_update(
    uint64_t bs_id, protocol::flex_lc_config_reply lc_config)
{
  protocol::flex_header *header1(new protocol::flex_header);
  header1->set_type(protocol::FLPT_GET_LC_CONFIG_REPLY);
  header1->set_version(0);
  header1->set_xid(0);

  protocol::flex_lc_config_reply *lc_config_reply_msg(new protocol::flex_lc_config_reply);
  lc_config_reply_msg->CopyFrom(lc_config);
  lc_config_reply_msg->set_allocated_header(header1);

  protocol::flexran_message message;
  message.set_msg_dir(protocol::INITIATING_MESSAGE);
  message.set_allocated_lc_config_reply_msg(lc_config_reply_msg);
  req_manager_.send_message(bs_id, message);
}

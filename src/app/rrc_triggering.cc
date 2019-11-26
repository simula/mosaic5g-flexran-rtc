/*
 * Copyright 2016-2019 FlexRAN Authors, Eurecom and The University of Edinburgh
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
 *  \brief   Configure RRC measurements
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include "rrc_triggering.h"
#include "rt_controller_common.h"
#include "flexran.pb.h"
#include <google/protobuf/util/json_util.h>
namespace proto_util = google::protobuf::util;

flexran::app::rrc::rrc_triggering::rrc_triggering(
    const rib::Rib& rib,
    const core::requests_manager& rm,
    event::subscription& sub)
  : component(rib, rm, sub)
{
  event_sub_.subscribe_bs_add(
      boost::bind(&flexran::app::rrc::rrc_triggering::bs_added, this, _1));
  event_sub_.subscribe_bs_remove(
      boost::bind(&flexran::app::rrc::rrc_triggering::bs_removed, this, _1));
}

void flexran::app::rrc::rrc_triggering::bs_added(uint64_t bs_id)
{
  /* Should check that physical cell IDs of all BSs are distinct and print a
   * warning otherwise. However, at this point there will be no configuration.
   * Hence, we trigger a tick event for later */
  set_check_phyCellId.insert(bs_id);
  if (!tick_check_phyCellId.connected())
    tick_check_phyCellId = event_sub_.subscribe_task_tick(
        boost::bind(&flexran::app::rrc::rrc_triggering::check_phyCellId, this, _1),
            10, event_sub_.last_tick());
}

void flexran::app::rrc::rrc_triggering::check_phyCellId(uint64_t tick)
{
  _unused(tick);
  for (auto it = set_check_phyCellId.begin(); it != set_check_phyCellId.end(); ) {
    const uint64_t bs_id = *it;
    const std::shared_ptr<flexran::rib::enb_rib_info> bs = rib_.get_bs(bs_id);
    if (bs
        && bs->get_enb_config().cell_config_size() > 0
        && bs->get_enb_config().cell_config(0).has_phy_cell_id()) {
      const int phyCellId = bs->get_enb_config().cell_config(0).phy_cell_id();
      for (auto o : map_phyCellId)
        if (phyCellId == o.second)
          LOG4CXX_WARN(flog::app, "rrc_triggering: New BS " << bs_id
              << " has the same phyCellId ("
              << phyCellId << ") as old BS " << o.first);
      /* we checked it against all BSs known to us */
      map_phyCellId.insert(std::make_pair(bs_id, phyCellId));
      it = set_check_phyCellId.erase(it);
    } else {
      it++;
    }
  }
  if (set_check_phyCellId.empty())
    tick_check_phyCellId.disconnect();
}

void flexran::app::rrc::rrc_triggering::bs_removed(uint64_t bs_id)
{
  const auto it = map_phyCellId.find(bs_id);
  if (it != map_phyCellId.end())
    map_phyCellId.erase(it);
}

bool flexran::app::rrc::rrc_triggering::rrc_reconf(const std::string& bs,
    const std::string& config,
    std::string& error_reason)
{
  uint64_t bs_id = parse_bs_id(bs);
  if (bs_id == 0) {
    error_reason = "can not find BS";
    return false;
  }

  protocol::flex_measurement_info rrc_info;
  auto ret = proto_util::JsonStringToMessage(config, &rrc_info, proto_util::JsonParseOptions());
  if (ret != proto_util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf  message:" << ret.ToString());
    return false;
  }

  /* validate here */
  //if (!verify_rrc_reconfig(rrc_info, error_reason)) {
  //  LOG4CXX_ERROR(flog::app, error_reason);
  //  return false;
  //}

  std::string s;
  proto_util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  proto_util::MessageToJsonString(rrc_info, &s, opt);
  LOG4CXX_INFO(flog::app, "sent new RRC measurement info to BS "
      << bs_id << ":\n" << s);
  push_config(bs_id, rrc_info);

  return true;
}

bool flexran::app::rrc::rrc_triggering::rrc_ho(
    const std::string& sbs, const std::string& ue, const std::string& tbs,
    std::string& error_reason)
{
  uint64_t s_bs_id  = rib_.parse_bs_id(sbs);
  /* if this is no BS ID, check whether this is a physical cell id */
  if (s_bs_id == 0)
    s_bs_id = parse_physical_cell_id(sbs);
  if (s_bs_id == 0) {
    /* could still not find */
    error_reason = "can not find source BS";
    return false;
  }

  /* if this BS is not under NetControl, reject this request */
  if (!rib_.get_bs(s_bs_id)->get_enb_config().cell_config(0).x2_ho_net_control()) {
    error_reason = "BS is not under NetControl";
    return false;
  }

  flexran::rib::rnti_t rnti;
  if (!rib_.get_bs(s_bs_id)->parse_rnti_imsi(ue, rnti)) {
    error_reason = "can not find UE";
    return false;
  }

  uint64_t t_bs_id  = rib_.parse_bs_id(tbs);
  /* if this is no BS ID, check whether this is a physical cell id */
  if (t_bs_id == 0)
    t_bs_id = parse_physical_cell_id(tbs);
  if (t_bs_id == 0) {
    /* could still not find */
    error_reason = "can not find target BS";
    return false;
  }
  if (s_bs_id == t_bs_id) {
    error_reason = "can not hand over to source BS (target is source)";
    return false;
  }
  uint32_t t_pc_id = rib_.get_bs(t_bs_id)->get_enb_config().cell_config(0).phy_cell_id();

  push_ho(s_bs_id, rnti, t_pc_id);
  uint32_t s_pc_id = rib_.get_bs(s_bs_id)->get_enb_config().cell_config(0).phy_cell_id();
  LOG4CXX_INFO(flog::app, "Sent hand-over command for RNTI " << rnti
      << ": source ID " << s_bs_id << ", PhysCellId " << s_pc_id << " -> "
      << " target ID " << t_bs_id << ", PhysCellId " << t_pc_id);
  return true;
}

bool flexran::app::rrc::rrc_triggering::rrc_x2_ho_net_control(
    const std::string& bs, bool x2_ho_net_control, std::string& error_reason)
{
  uint64_t bs_id = rib_.parse_enb_agent_id(bs);
  if (bs_id == 0) {
    error_reason = "can not find BS";
    return false;
  }

  push_x2_ho_net_control(bs_id, x2_ho_net_control);
  LOG4CXX_INFO(flog::app, "Sent new X2 Handover policy: "
      << (x2_ho_net_control ? "network-initiated" : "UE-initiated"));
  return true;
}

void flexran::app::rrc::rrc_triggering::push_config(
    uint64_t bs_id, const protocol::flex_measurement_info& rrc_info)
{
  protocol::flexran_message config_message;
  // Create control delegation message header
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_RRC_TRIGGERING);
  header->set_version(0);
  header->set_xid(0);
  
  protocol::flex_rrc_triggering *rrc_triggering(new protocol::flex_rrc_triggering);
  rrc_triggering->set_allocated_header(header);
  rrc_triggering->mutable_meas_info()->CopyFrom(rrc_info);

  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_rrc_triggering(rrc_triggering);
  req_manager_.send_message(bs_id, config_message);
}

void flexran::app::rrc::rrc_triggering::push_ho(
    uint64_t s_bs_id, flexran::rib::rnti_t rnti, uint32_t target_phy_cell_id)
{
  protocol::flexran_message message;
  // Create control delegation message header
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_HO_COMMAND);
  header->set_version(0);
  header->set_xid(0);

  protocol::flex_ho_command *ho_command(new protocol::flex_ho_command);
  ho_command->set_allocated_header(header);
  ho_command->set_rnti(rnti);
  ho_command->set_target_phy_cell_id(target_phy_cell_id);

  message.set_msg_dir(protocol::INITIATING_MESSAGE);
  message.set_allocated_ho_command(ho_command);
  req_manager_.send_message(s_bs_id, message);
}

void flexran::app::rrc::rrc_triggering::push_x2_ho_net_control(
    uint64_t bs_id, bool x2_ho_net_control)
{
  protocol::flexran_message message;
  // Create control delegation message header
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_GET_ENB_CONFIG_REPLY);
  header->set_version(0);
  header->set_xid(0);

  protocol::flex_enb_config_reply *enb_config(new protocol::flex_enb_config_reply);
  enb_config->add_cell_config();
  enb_config->mutable_cell_config(0)->set_x2_ho_net_control(x2_ho_net_control);

  message.set_msg_dir(protocol::INITIATING_MESSAGE);
  message.set_allocated_enb_config_reply_msg(enb_config);
  req_manager_.send_message(bs_id, message);
}

uint64_t flexran::app::rrc::rrc_triggering::parse_bs_agent_id(const std::string& s) const
{
  if (s.empty())
    return rib_.parse_enb_agent_id("-1");
  return rib_.parse_enb_agent_id(s);
}

uint64_t flexran::app::rrc::rrc_triggering::parse_bs_id(const std::string& s) const
{
  if (s.empty())
    return rib_.parse_enb_agent_id("-1");
  return rib_.parse_bs_id(s);
}

uint64_t flexran::app::rrc::rrc_triggering::parse_physical_cell_id(const std::string& s) const
{
  uint32_t pc_id;
  try {
    pc_id = std::stoi(s);
  } catch (const std::invalid_argument& e) {
    return 0;
  }
  if (pc_id > 503) /* see TS 36.331, Sec.6-3-4 PhysCellId */
    return 0;
  for (uint64_t bs : rib_.get_available_base_stations()) {
    auto bs_ptr = rib_.get_bs(bs);
    if (pc_id == bs_ptr->get_enb_config().cell_config(0).phy_cell_id())
      return bs;
  }
  return 0;
}

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

/*! \file    rib_updater.cc
 *  \brief   dispatcher of protobuf messages, maintains RIB
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include <iostream>

#include "rib_updater.h"
#include "tagged_message.h"
#include "rt_wrapper.h"
#include "enb_rib_info.h"

#include "rt_controller_common.h"

#include "flexran_log.h"

#ifdef PROFILE
#include <iostream>
extern std::atomic_bool g_doprof;
#endif

unsigned int flexran::rib::rib_updater::run()
{
  return update_rib();
}

#ifdef PROFILE
void flexran::rib::rib_updater::print_prof_results(std::chrono::duration<double> d)
{
  double us = std::chrono::duration_cast<std::chrono::microseconds>(d).count();
  std::cout << "*** Agent throughput profiling results during "
      << us << "us ***\n";
  for (auto a : rib_.get_agents()) {
    std::cout << "agent " << a.second->agent_id << " BS " << a.second->bs_id
        << " rx packets " << a.second->rx_packets
        << " rx_bytes " << a.second->rx_bytes
        << " ~Mbps " << static_cast<double>(a.second->rx_bytes) / us << std::endl;
    a.second->rx_bytes = 0;
    a.second->rx_packets = 0;
  }
}
#endif

unsigned int flexran::rib::rib_updater::update_rib()
{
  unsigned int processed = 0;
  int rem_msgs = messages_to_check_;
  std::shared_ptr<flexran::network::tagged_message> tm;

  while(net_xface_.get_msg_from_network(tm) && (rem_msgs > 0)) {
    if (tm->getSize() == 0) { // New connection. update the pending eNBs list
      handle_new_connection(tm->getTag());
    } else {
#ifdef PROFILE
      if (g_doprof) {
        std::shared_ptr<agent_info> a = rib_.get_agent(tm->getTag());
        a->rx_packets++;
        a->rx_bytes += tm->getSize();
      }
#endif
      dispatch_message(tm);
    }
    rem_msgs--;
    processed++;
  }
  return processed;
}

void flexran::rib::rib_updater::handle_new_connection(int agent_id)
{
  LOG4CXX_INFO(flog::rib, "New agent connection established (agent ID "
      << agent_id << "), sending hello");
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_HELLO);
  header->set_version(0);
  header->set_xid(0);

  protocol::flex_hello *hello_msg(new protocol::flex_hello);
  hello_msg->set_allocated_header(header);

  protocol::flexran_message out_message;
  out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message.set_allocated_hello_msg(hello_msg);
  net_xface_.send_msg(out_message, agent_id);
}

void flexran::rib::rib_updater::dispatch_message(std::shared_ptr<flexran::network::tagged_message> tm)
{
  protocol::flexran_message in_message;

  // Deserialize the message
  in_message.ParseFromArray(tm->getMessageContents(), tm->getSize());
  // Update the RIB based on the message type
  switch (in_message.msg_case()) {
  case protocol::flexran_message::kHelloMsg:
    handle_hello(tm->getTag(), in_message.hello_msg(), in_message.msg_dir());
    break;
  case protocol::flexran_message::kEchoRequestMsg:
    handle_echo_request(tm->getTag(), in_message.echo_request_msg());
    break;
  case protocol::flexran_message::kEchoReplyMsg:
    handle_echo_reply(tm->getTag(), in_message.echo_reply_msg());
    break;
  case protocol::flexran_message::kStatsReplyMsg:
    handle_stats_reply(tm->getTag(), in_message.stats_reply_msg());
    break;
  case protocol::flexran_message::kSfTriggerMsg:
    handle_sf_trigger(tm->getTag(), in_message.sf_trigger_msg());
    break;
  case protocol::flexran_message::kUlSrInfoMsg:
    LOG4CXX_WARN(flog::rib, "NOT IMPLEMENTED Agent " << tm->getTag()
                 << ": received UL sr info msg");
    break;
  case protocol::flexran_message::kEnbConfigReplyMsg:
    handle_enb_config_reply(tm->getTag(), in_message.enb_config_reply_msg());
    break;
  case protocol::flexran_message::kUeConfigReplyMsg:
    handle_ue_config_reply(tm->getTag(), in_message.ue_config_reply_msg());
    break;
  case protocol::flexran_message::kLcConfigReplyMsg:
    handle_lc_config_reply(tm->getTag(), in_message.lc_config_reply_msg());
    break;
  case protocol::flexran_message::kUeStateChangeMsg:
    handle_ue_state_change(tm->getTag(), in_message.ue_state_change_msg());
    break;
  case protocol::flexran_message::kDisconnectMsg:
    handle_disconnect(tm->getTag(), in_message.disconnect_msg());
    break;
  default:
    LOG4CXX_WARN(flog::rib, "UNKNOWN MESSAGE from Agent " << tm->getTag());
    break;
  }
}

// Handle hello message
void flexran::rib::rib_updater::handle_hello(int agent_id,
    const protocol::flex_hello& hello_msg,
    protocol::flexran_direction dir)
{
  // copy message to support old agents which do not have BS ID and/or
  // capabilities, so here we fill it and then add the agent
  protocol::flex_hello hello_copy = hello_msg;
  if (dir != protocol::SUCCESSFUL_OUTCOME) {
    LOG4CXX_WARN(flog::rib, "Agent " << agent_id
        << ", hello msg not successful");
    net_xface_.release_connection(agent_id);
    return;
  }
  if (hello_copy.bs_id() == 0) {
    hello_copy.set_bs_id(BS_ID_OFFSET + agent_id);
    LOG4CXX_WARN(flog::rib, "Agent " << agent_id
        << " with illegal BS ID 0, assigned BS ID " << hello_copy.bs_id());
  }
  if (hello_copy.capabilities_size() == 0) {
    hello_copy.add_capabilities(protocol::flex_bs_capability::LOPHY);
    hello_copy.add_capabilities(protocol::flex_bs_capability::HIPHY);
    hello_copy.add_capabilities(protocol::flex_bs_capability::LOMAC);
    hello_copy.add_capabilities(protocol::flex_bs_capability::HIMAC);
    hello_copy.add_capabilities(protocol::flex_bs_capability::RLC);
    hello_copy.add_capabilities(protocol::flex_bs_capability::PDCP);
    hello_copy.add_capabilities(protocol::flex_bs_capability::RRC);
    hello_copy.add_capabilities(protocol::flex_bs_capability::SDAP);
    LOG4CXX_WARN(flog::rib, "Agent " << agent_id
        << " with empty capabilities, assuming full capabilities");
  }

  std::shared_ptr<agent_info> agent = std::make_shared<agent_info>(
      agent_id, hello_copy.bs_id(), hello_copy.capabilities(),
      net_xface_.get_endpoint(agent_id));
  LOG4CXX_INFO(flog::rib, "Agent " << agent_id << ": hello BS "
      << agent->bs_id << ", capabilities " << agent->capabilities.to_string());

  if (rib_.get_bs(agent->bs_id) != nullptr) {
    LOG4CXX_ERROR(flog::rib, "BS with ID " << agent->bs_id
        << " already exists, aborting connection");
    net_xface_.release_connection(agent_id);
    return;
  }
  
  if (!rib_.add_pending_agent(agent)) {
    LOG4CXX_ERROR(flog::rib, "Could not add agent " << agent_id
        << ", aborting connection");
    net_xface_.release_connection(agent_id);
    return;
  }
  if (rib_.new_eNB_config_entry(agent->bs_id)) {
    // new BS is complete -> ask for configuration
    LOG4CXX_INFO(flog::rib, "New BS " << agent->bs_id
        << ", creating RIB entry");
    trigger_bs_config(agent->bs_id);
    event_sub_.bs_add_(agent->bs_id);
  } else {
    LOG4CXX_WARN(flog::rib, "Could create BS " << agent->bs_id << " (yet)");
  }
}

void flexran::rib::rib_updater::handle_echo_request(int agent_id,
    const protocol::flex_echo_request& echo_request_msg)
{
  LOG4CXX_INFO(flog::rib, "Agent " << agent_id << ": received echo request msg");
  // Need to send an echo reply
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_ECHO_REPLY);
  header->set_version(0);
  header->set_xid(echo_request_msg.header().xid());

  protocol::flex_echo_reply *echo_reply_msg(new protocol::flex_echo_reply);
  echo_reply_msg->set_allocated_header(header);

  protocol::flexran_message out_message;
  out_message.set_msg_dir(protocol::SUCCESSFUL_OUTCOME);
  out_message.set_allocated_echo_reply_msg(echo_reply_msg);
  net_xface_.send_msg(out_message, agent_id);
}

void flexran::rib::rib_updater::handle_echo_reply(int agent_id,
    const protocol::flex_echo_reply& echo_reply_msg)
{
  _unused(echo_reply_msg);  
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs && !rib_.agent_is_pending(agent_id)) {
    warn_unknown_agent_bs(__func__, agent_id);
    return;
  }

  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received echo reply msg");
  if (bs) bs->update_liveness();
}

void flexran::rib::rib_updater::handle_sf_trigger(int agent_id,
    const protocol::flex_sf_trigger& sf_trigger_msg)
{
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs) {
    warn_unknown_agent_bs(__func__, agent_id);
    return;
  }

  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << "/BS "
      << bs->get_id() << " received a subframe trigger msg");
  bs->update_subframe(sf_trigger_msg);
}

void flexran::rib::rib_updater::handle_enb_config_reply(int agent_id,
    const protocol::flex_enb_config_reply& enb_config_reply_msg)
{
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs) {
    warn_unknown_agent_bs(__func__, agent_id);
    return;
  }

  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << " received an eNB config reply msg");
  bs->update_eNB_config(enb_config_reply_msg);
}

void flexran::rib::rib_updater::handle_ue_config_reply(int agent_id,
    const protocol::flex_ue_config_reply& ue_config_reply_msg)
{
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs) {
    warn_unknown_agent_bs(__func__, agent_id);
    return;
  }

  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << " received a UE config reply msg");
  bs->update_UE_config(ue_config_reply_msg);
}

void flexran::rib::rib_updater::handle_lc_config_reply(int agent_id,
    const protocol::flex_lc_config_reply& lc_config_reply_msg)
{
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs) {
    LOG4CXX_WARN(flog::rib, "handle_lc_config_reply(): "
        << "BS unknown for agent " << agent_id);
    return;
  }

  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received an LC config reply msg");
  bs->update_LC_config(lc_config_reply_msg);
}

void flexran::rib::rib_updater::handle_stats_reply(int agent_id,
    const protocol::flex_stats_reply& mac_stats_reply)
{
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs) {
    warn_unknown_agent_bs(__func__, agent_id);
    protocol::flex_header *header(new protocol::flex_header);
    header->set_type(protocol::FLPT_STATS_REQUEST);
    header->set_version(0);
    header->set_xid(0);
    protocol::flex_complete_stats_request *off_msg(new protocol::flex_complete_stats_request);
    off_msg->set_report_frequency(protocol::FLSRF_OFF);
    protocol::flex_stats_request *stats_request_msg(new protocol::flex_stats_request);
    stats_request_msg->set_allocated_header(header);
    stats_request_msg->set_type(protocol::FLST_COMPLETE_STATS);
    stats_request_msg->set_allocated_complete_stats_request(off_msg);
    protocol::flexran_message out_message;
    out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
    out_message.set_allocated_stats_request_msg(stats_request_msg);
    net_xface_.send_msg(out_message, agent_id);
    return;
  }

  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received stats reply msg");
  bs->update_mac_stats(mac_stats_reply);
}

void flexran::rib::rib_updater::handle_ue_state_change(int agent_id,
    const protocol::flex_ue_state_change& ue_state_change_msg)
{
  auto bs = rib_.get_bs_from_agent(agent_id);
  if (!bs) {
    warn_unknown_agent_bs(__func__, agent_id);
    return;
  }

  bs->update_UE_config(ue_state_change_msg);
  switch (ue_state_change_msg.type()) {
  case protocol::FLUESC_ACTIVATED:
    event_sub_.ue_connect_(bs->get_id(), ue_state_change_msg.config().rnti());
    trigger_bs_config(bs->get_id());
    break;
  case protocol::FLUESC_UPDATED:
    event_sub_.ue_update_(bs->get_id(), ue_state_change_msg.config().rnti());
    break;
  case protocol::FLUESC_DEACTIVATED:
    event_sub_.ue_disconnect_(bs->get_id(), ue_state_change_msg.config().rnti());
    break;
  default:
    break;
  }
}

void flexran::rib::rib_updater::handle_disconnect(int agent_id,
    const protocol::flex_disconnect& disconnect_msg)
{
  _unused(disconnect_msg);
  uint64_t bs_id = rib_.get_bs_id(agent_id);
  if (bs_id > 0) { // existing base station
    LOG4CXX_INFO(flog::rib, "Agent " << agent_id << " of BS " << bs_id << " disconnected");
    if (rib_.remove_eNB_config_entry(agent_id)) {
      LOG4CXX_INFO(flog::rib, "BS " << bs_id << " is offline");
      event_sub_.bs_remove_(bs_id);
    }
  } else if (rib_.agent_is_pending(agent_id)) { // pending base station
    LOG4CXX_INFO(flog::rib, "Pending agent " << agent_id << " disconnected");
    rib_.remove_pending_agent(agent_id);
  } else { // unknown agent
    warn_unknown_agent_bs(__func__, agent_id);
  }
  net_xface_.release_connection(agent_id);
}

void flexran::rib::rib_updater::trigger_bs_config(uint64_t bs_id)
{
  // BS is alive. Request info about its configuration (from all agents)
  // eNB config first
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

  // UE config second
  protocol::flex_header *header2(new protocol::flex_header);
  header2->set_type(protocol::FLPT_GET_UE_CONFIG_REQUEST);
  header2->set_version(0);
  header2->set_xid(1);
  protocol::flex_ue_config_request *ue_config_request_msg(new protocol::flex_ue_config_request);
  ue_config_request_msg->set_allocated_header(header2);
  protocol::flexran_message out_message2;
  out_message2.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message2.set_allocated_ue_config_request_msg(ue_config_request_msg);
  req_manager_.send_message(bs_id, out_message2);

  // LC config third
  protocol::flex_header *header3(new protocol::flex_header);
  header3->set_type(protocol::FLPT_GET_LC_CONFIG_REQUEST);
  header2->set_version(0);
  header3->set_xid(2);
  protocol::flex_lc_config_request *lc_config_request_msg(new protocol::flex_lc_config_request);
  lc_config_request_msg->set_allocated_header(header3);
  protocol::flexran_message out_message3;
  out_message3.set_msg_dir(protocol::INITIATING_MESSAGE);
  out_message3.set_allocated_lc_config_request_msg(lc_config_request_msg);
  req_manager_.send_message(bs_id, out_message3);
}

void flexran::rib::rib_updater::warn_unknown_agent_bs(const std::string& function, int agent_id)
{
  LOG4CXX_WARN(flog::rib, function << "(): unknown BS for agent " << agent_id);
}

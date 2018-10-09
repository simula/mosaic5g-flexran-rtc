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

#include "rt_controller_common.h"

#include "flexran_log.h"

unsigned int flexran::rib::rib_updater::run()
{
  return update_rib();
}

unsigned int flexran::rib::rib_updater::update_rib()
{
  unsigned int processed = 0;
  int rem_msgs = messages_to_check_;
  protocol::flexran_message in_message;
  std::shared_ptr<flexran::network::tagged_message> tm;

  while(net_xface_.get_msg_from_network(tm) && (rem_msgs > 0)) {
    if (tm->getSize() == 0) { // New connection. update the pending eNBs list
      handle_new_connection(tm->getTag());
      // TODO add once we have the necessary information
      //rib_.add_pending_agent(tm->getTag());
      continue;
    } else { // Message from existing connection. Just update the proper rib entries
      // Deserialize the message
      in_message.ParseFromArray(tm->getMessageContents(), tm->getSize());
      // Update the RIB based on the message type
      if(in_message.has_hello_msg()) {
        handle_hello(tm->getTag(), in_message.hello_msg(), in_message.msg_dir());
      } else if(in_message.has_echo_request_msg()) {
        handle_echo_request(tm->getTag(), in_message.echo_request_msg());
      } else if(in_message.has_echo_reply_msg()) {
        handle_echo_reply(tm->getTag(), in_message.echo_reply_msg());
      } else if(in_message.has_stats_reply_msg()) {
        handle_stats_reply(tm->getTag(), in_message.stats_reply_msg());
      } else if(in_message.has_sf_trigger_msg()) {
        handle_sf_trigger(tm->getTag(), in_message.sf_trigger_msg());
      } else if(in_message.has_ul_sr_info_msg()) {
        /* TODO: Need to implement to enable UL scheduling */
        LOG4CXX_WARN(flog::rib, "NOT IMPLEMENTED Agent " << tm->getTag() << ": received UL sr info msg");
      } else if(in_message.has_enb_config_reply_msg()) {
        handle_enb_config_reply(tm->getTag(), in_message.enb_config_reply_msg());
      } else if(in_message.has_ue_config_reply_msg()) {
        handle_ue_config_reply(tm->getTag(), in_message.ue_config_reply_msg());
      } else if(in_message.has_lc_config_reply_msg()) {
        handle_lc_config_reply(tm->getTag(), in_message.lc_config_reply_msg());
      } else if(in_message.has_ue_state_change_msg()) {
        handle_ue_state_change(tm->getTag(), in_message.ue_state_change_msg());
      } else if(in_message.has_disconnect_msg()) {
        handle_disconnect(tm->getTag(), in_message.disconnect_msg());
      } else {
        LOG4CXX_WARN(flog::rib, "UNKNOWN MESSAGE from Agent " << tm->getTag());
      }
    }
    rem_msgs--;
    processed++;
  }
  return processed;
}

void flexran::rib::rib_updater::handle_new_connection(int agent_id)
{
  LOG4CXX_INFO(flog::rib, "New agent connection established (agent ID " << agent_id << "), sending hello");
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

// Handle hello message
void flexran::rib::rib_updater::handle_hello(int agent_id,
    const protocol::flex_hello& hello_msg,
    protocol::flexran_direction dir)
{
  LOG4CXX_INFO(flog::rib, "Agent " << agent_id << ": hello msg");
  _unused(hello_msg);
  
  if (dir == protocol::SUCCESSFUL_OUTCOME) {
    // Agent is alive. Request info about its configuration
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
    net_xface_.send_msg(out_message1, agent_id);
  
    // UE config second
    protocol::flexran_message out_message2;
    out_message2.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::flex_header *header2(new protocol::flex_header);
    header2->set_type(protocol::FLPT_GET_UE_CONFIG_REQUEST);
    header2->set_xid(1);
    protocol::flex_ue_config_request *ue_config_request_msg(new protocol::flex_ue_config_request);
    ue_config_request_msg->set_allocated_header(header2);
    out_message2.set_allocated_ue_config_request_msg(ue_config_request_msg);
    net_xface_.send_msg(out_message2, agent_id);

    // LC config third
    protocol::flexran_message out_message3;
    out_message3.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::flex_header *header3(new protocol::flex_header);
    header3->set_type(protocol::FLPT_GET_LC_CONFIG_REQUEST);
    header3->set_xid(2);
    protocol::flex_lc_config_request *lc_config_request_msg(new protocol::flex_lc_config_request);
    lc_config_request_msg->set_allocated_header(header3);
    out_message3.set_allocated_lc_config_request_msg(lc_config_request_msg);
    net_xface_.send_msg(out_message3, agent_id);
  } // Hello should originate from controller - Ignore in all other cases
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

  if (rib_.has_eNB_config_entry(agent_id)) {
    LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received echo reply msg");
    rib_.update_liveness(agent_id);
  } else {
    LOG4CXX_WARN(flog::rib, "handle_echo_reply(): unknown agent "
        << agent_id << " for echo_reply_msg");
    /* TODO: Should probably do some error handling */
  }
}

void flexran::rib::rib_updater::handle_sf_trigger(int agent_id,
    const protocol::flex_sf_trigger& sf_trigger_msg)
{
  if (rib_.has_eNB_config_entry(agent_id)) {
    LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received a subframe trigger msg");
    rib_.set_subframe_updates(agent_id, sf_trigger_msg);
  } else {
    LOG4CXX_WARN(flog::rib, "handle_sf_trigger(): unknown agent "
        << agent_id << " for sf_trigger_msg");
    /* TODO: Should probably do some error handling */
  }
}

void flexran::rib::rib_updater::handle_enb_config_reply(int agent_id,
    const protocol::flex_enb_config_reply& enb_config_reply_msg)
{
  LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received an eNB config reply msg");
  if (rib_.agent_is_pending(agent_id)) {
    // Must create a new eNB_config entry
    rib_.new_eNB_config_entry(agent_id);
    rib_.remove_pending_agent(agent_id);
    LOG4CXX_INFO(flog::rib, "Agent " << agent_id << " with ID "
        << "REPLACE" << std::hex << " (0x"
        << "REPLACE"
        << ") was pending, created contiguration entry");
  }// If agent was not pending we should ignore this message. Only for initialization

  rib_.eNB_config_update(agent_id, enb_config_reply_msg);
}

void flexran::rib::rib_updater::handle_ue_config_reply(int agent_id,
    const protocol::flex_ue_config_reply& ue_config_reply_msg)
{
  if (rib_.has_eNB_config_entry(agent_id)) {
    LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received a UE config reply msg");
    rib_.ue_config_update(agent_id, ue_config_reply_msg);
  } else {
    LOG4CXX_WARN(flog::rib, "handle_lc_config_reply(): unknown agent "
        << agent_id << " for ue_config_reply_msg");
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_lc_config_reply(int agent_id,
    const protocol::flex_lc_config_reply& lc_config_reply_msg)
{
  if(rib_.has_eNB_config_entry(agent_id)) {
    LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received an LC config reply msg");
    rib_.lc_config_update(agent_id, lc_config_reply_msg);
  } else {
    LOG4CXX_WARN(flog::rib, "handle_lc_config_reply(): unknown agent "
        << agent_id << " for lc_config_reply_msg");
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_stats_reply(int agent_id,
    const protocol::flex_stats_reply& mac_stats_reply)
{
  if(rib_.has_eNB_config_entry(agent_id)) {
    LOG4CXX_DEBUG(flog::rib, "Agent " << agent_id << ": received stats reply msg");
    rib_.mac_stats_update(agent_id, mac_stats_reply);
  } else {
    LOG4CXX_WARN(flog::rib, "handle_message(): unknown agent "
        << agent_id << " for mac_stats_reply");
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_ue_state_change(int agent_id,
    const protocol::flex_ue_state_change& ue_state_change_msg)
{
  if(rib_.has_eNB_config_entry(agent_id)) {
    /* TODO add the handler for the update of the state */
    LOG4CXX_INFO(flog::rib, "Agent " << agent_id << ": UE state changed");
    rib_.ue_config_update(agent_id, ue_state_change_msg);
    protocol::flexran_message out_message;
    out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::flex_header *header(new protocol::flex_header);
    header->set_type(protocol::FLPT_GET_LC_CONFIG_REQUEST);
    header->set_xid(2);
    protocol::flex_lc_config_request *lc_config_request_msg(new protocol::flex_lc_config_request);
    lc_config_request_msg->set_allocated_header(header);
    out_message.set_allocated_lc_config_request_msg(lc_config_request_msg);
    net_xface_.send_msg(out_message, agent_id);
  } else {
    LOG4CXX_WARN(flog::rib, "handle_message(): unknown agent "
        << agent_id << " for ue_state_change_msg");
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_disconnect(int agent_id,
    const protocol::flex_disconnect& disconnect_msg)
{
  _unused(disconnect_msg);
  if(rib_.has_eNB_config_entry(agent_id)) {
    LOG4CXX_INFO(flog::rib, "Agent " << agent_id << " disconnected");
    rib_.remove_eNB_config_entry(agent_id);
    net_xface_.release_connection(agent_id);
  } /* else: nothing to do, ignore */
}

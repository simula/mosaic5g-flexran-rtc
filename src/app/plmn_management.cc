/*
 * Copyright 2016-2020 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    plmn_management.cc
 *  \brief   app manages a BS's PLMN and MME/control CN node
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <google/protobuf/util/json_util.h>
namespace proto_util = google::protobuf::util;

#include "plmn_management.h"
#include "flexran.pb.h"

#include "flexran_log.h"

flexran::app::management::plmn_management::plmn_management(rib::Rib& rib,
    const core::requests_manager& rm, event::subscription& sub)
  : component(rib, rm, sub)
{
}

void flexran::app::management::plmn_management::add_mme(const std::string& bs,
    const std::string& config)
{
  uint64_t bs_id = rib_.parse_bs_id(bs);
  if (bs_id == 0)
    throw std::invalid_argument("cannot find BS " + bs);

  protocol::flex_s1ap_config s1ap;
  auto ret = proto_util::JsonStringToMessage(config, &s1ap, proto_util::JsonParseOptions());
  if (ret != proto_util::Status::OK) {
    LOG4CXX_ERROR(flog::app, "error while parsing ProtoBuf message:" << ret.ToString());
    throw std::invalid_argument("Protobuf parser error");
  }

  s1ap.clear_pending();
  s1ap.clear_connected();
  s1ap.clear_enb_s1_ip();
  s1ap.clear_enb_name();
  if (s1ap.mme_size() == 0)
    throw std::invalid_argument("no MME present");
  if (s1ap.mme_size() > 1)
    throw std::invalid_argument("only one MME allowed");

  for (protocol::flex_s1ap_mme& mme : *s1ap.mutable_mme()) {
    if (!mme.has_s1_ip())
      throw std::invalid_argument("no S1Ip for MME");
    const std::string& ip = mme.s1_ip();
    const auto bs = rib_.get_bs(bs_id);
    const auto s1ap = bs->get_enb_config().s1ap();
    bool ip_present = std::any_of(s1ap.mme().begin(), s1ap.mme().end(),
        [ip](const protocol::flex_s1ap_mme& mme) { return mme.s1_ip() == ip; });
    if (ip_present)
      throw std::invalid_argument("MME at IP " + ip + " already present");
    mme.clear_state();
    mme.clear_served_gummeis();
    if (mme.requested_plmns_size() > 0)
      throw std::invalid_argument("requested PLMNs not supported yet");
    mme.clear_rel_capacity();
  }

  std::string s;
  proto_util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  proto_util::MessageToJsonString(s1ap, &s, opt);
  LOG4CXX_INFO(flog::app, "sending MME list to BS " << bs_id << ":\n" << s);

  push_mme_config(bs_id, s1ap);
}

void flexran::app::management::plmn_management::remove_mme(const std::string& bs,
    const std::string& config)
{
  uint64_t bs_id = rib_.parse_bs_id(bs);
  if (bs_id == 0)
    throw std::invalid_argument("cannot find BS " + bs);

  protocol::flex_s1ap_config s1ap;
  auto ret = proto_util::JsonStringToMessage(config, &s1ap, proto_util::JsonParseOptions());
  if (ret != proto_util::Status::OK) {
    LOG4CXX_ERROR(flog::app, "error while parsing ProtoBuf message:" << ret.ToString());
    throw std::invalid_argument("Protobuf parser error");
  }

  s1ap.clear_pending();
  s1ap.clear_connected();
  s1ap.clear_enb_s1_ip();
  s1ap.clear_enb_name();
  if (s1ap.mme_size() == 0)
    throw std::invalid_argument("no MME present");
  if (s1ap.mme_size() > 1)
    throw std::invalid_argument("only one MME allowed");

  for (protocol::flex_s1ap_mme& mme : *s1ap.mutable_mme()) {
    if (!mme.has_s1_ip())
      throw std::invalid_argument("no S1Ip for MME");
    const std::string& ip = mme.s1_ip();
    const auto bs = rib_.get_bs(bs_id);
    const auto s1ap = bs->get_enb_config().s1ap();
    bool ip_present = std::any_of(s1ap.mme().begin(), s1ap.mme().end(),
        [ip](const protocol::flex_s1ap_mme& mme) { return mme.s1_ip() == ip; });
    if (!ip_present)
      throw std::invalid_argument("no MME at IP " + ip + " present");
    mme.set_state(protocol::FLMMES_DISCONNECTED);
    mme.clear_served_gummeis();
    mme.clear_requested_plmns();
    mme.clear_rel_capacity();
  }

  std::string s;
  proto_util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  proto_util::MessageToJsonString(s1ap, &s, opt);
  LOG4CXX_INFO(flog::app, "sending MME list to BS " << bs_id << ":\n" << s);

  push_mme_config(bs_id, s1ap);
}

void flexran::app::management::plmn_management::push_mme_config(
    uint64_t bs_id, const protocol::flex_s1ap_config& s1ap)
{
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);

  protocol::flex_s1ap_config *s(new protocol::flex_s1ap_config);
  s->CopyFrom(s1ap);
  protocol::flex_enb_config_reply *enb_config_msg(new protocol::flex_enb_config_reply);
  enb_config_msg->set_allocated_s1ap(s);
  enb_config_msg->set_allocated_header(config_header);

  protocol::flexran_message config_message;
  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_enb_config_reply_msg(enb_config_msg);
  req_manager_.send_message(bs_id, config_message);
}

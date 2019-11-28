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

/*! \file    rrm_management.cc
 *  \brief   app is RRM calls helper (slice configuration, restart)
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <map>
#include <cmath>

#include <google/protobuf/util/json_util.h>

#include "rrm_management.h"
#include "flexran.pb.h"
#include "rib_common.h"
#include "band_check.h"

#include "flexran_log.h"

flexran::app::management::rrm_management::rrm_management(rib::Rib& rib,
    const core::requests_manager& rm, event::subscription& sub)
  : component(rib, rm, sub)
{
  event_sub_.subscribe_ue_connect(
    boost::bind(&flexran::app::management::rrm_management::ue_add_update, this, _1, _2));
  event_sub_.subscribe_ue_update(
    boost::bind(&flexran::app::management::rrm_management::ue_add_update, this, _1, _2));
}

bool flexran::app::management::rrm_management::is_free_common_slice_id(int slice_id) const
{
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<flexran::rib::enb_rib_info> bs_info = rib_.get_bs(bs_id);
    if (!bs_info) return false;
    if (!bs_info->get_enb_config().cell_config(0).has_slice_config()) return false;
    if (bs_info->has_dl_slice(slice_id)) return false;
    if (bs_info->has_ul_slice(slice_id)) return false;
  }
  return true;
}

int flexran::app::management::rrm_management::calculate_rbs_percentage(int bw, uint64_t bps)
{
  double pct;
  switch (bw) {
  case 25: /* experience shows we reach (almost) 17Mbps */
    if (bps > 17000000) return -1;
    pct = bps * 100.0 / 17000000.0;
    break;
  case 50: /* experience shows we reach (almost) 35Mbps */
    if (bps > 35000000) return -1;
    pct = bps * 100.0 / 35000000.0;
    break;
  default:
    return -1;
  }
  /* round up to the next full RB */
  return std::ceil(pct * bw / 100.0) * 100 / bw;
}

int flexran::app::management::rrm_management::instantiate_vnetwork(
    uint64_t bps, std::string& error_reason)
{
  /* first: find suitable slice ID for all BS */
  int slice_id;
  const int max_slice_id = 256;
  for (slice_id = 1; slice_id < max_slice_id; ++slice_id)
    if (is_free_common_slice_id(slice_id)) break;

  if (slice_id >= max_slice_id) {
    error_reason = "no free common slice ID found";
    return -1;
  }

  std::map<int, int> bs_pct;
  std::map<int, int> bs_first_rb;
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<flexran::rib::enb_rib_info> bs = rib_.get_bs(bs_id);
    /* calculate the necessary percentage of RBs */
    const int bw = bs->get_enb_config().cell_config(0).dl_bandwidth();
    const int pct = calculate_rbs_percentage(bw, bps);
    if (pct <= 0) {
      error_reason = "cannot calculate slice percentage for BS " + std::to_string(bs_id);
      return -1;
    }
    /* check that slice 0 percentage > new slice percentage and has at least 3% */
    const int sldl0pct = bs->get_enb_config().cell_config(0).slice_config().dl(0).percentage();
    const int slul0pct = bs->get_enb_config().cell_config(0).slice_config().ul(0).percentage();
    if (sldl0pct <= pct || slul0pct <= pct || (sldl0pct - pct) < 3 || (slul0pct - pct) < 3) {
      error_reason = "BS " + std::to_string(bs_id) + " cannot provide the requested bitrate";
      return -1;
    }
    /* calculate first_rb for UL, which is at end of slice 0 */
    const int first_rb0 = bs->get_enb_config().cell_config(0).slice_config().ul(0).first_rb();
    const int new_first_rb = first_rb0 + (slul0pct - pct) * bw / 100;
    bs_pct.emplace(bs_id, pct);
    bs_first_rb.emplace(bs_id, new_first_rb);
  }

  /* all BS can create such a slice, send a corresponding command to all */
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<flexran::rib::enb_rib_info> bs = rib_.get_bs(bs_id);
    const int sldl0pct = bs->get_enb_config().cell_config(0).slice_config().dl(0).percentage();
    const int slul0pct = bs->get_enb_config().cell_config(0).slice_config().ul(0).percentage();
    const std::string newsldl0pcts = std::to_string(sldl0pct - bs_pct.at(bs_id));
    const std::string newslul0pcts = std::to_string(slul0pct - bs_pct.at(bs_id));
    const std::string newpcts = std::to_string(bs_pct.at(bs_id));
    const std::string newfrbs = std::to_string(bs_first_rb.at(bs_id));
    /* This could be optimized by directly creating the protobuf structure */
    const std::string p = "{\"intrasliceShareActive\":false,"
          "\"intersliceShareActive\":false,"
          "\"dl\":[{id:0,percentage:" + newsldl0pcts
        + "},{id:" + std::to_string(slice_id) + ",\"percentage\":" + newpcts
        + "}],\"ul\":[{id:0,percentage:" + newslul0pcts + "},{id:" + std::to_string(slice_id)
        + ",\"percentage\":" + newpcts + ",\"first_rb\":" + newfrbs + "}]}";
    if (!apply_slice_config_policy(bs_id, p, error_reason))
      return -1;
  }
  return slice_id;
}

bool flexran::app::management::rrm_management::remove_vnetwork(
    uint32_t slice_id, std::string& error_reason)
{
  /* verify that all connected BSs have this slice in UL&DL */
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<flexran::rib::enb_rib_info> bs = rib_.get_bs(bs_id);
    if (!bs->has_dl_slice(slice_id) || !bs->has_ul_slice(slice_id)) {
      error_reason = "BS " + std::to_string(bs_id)
          + " does not have slice " + std::to_string(slice_id);
      return false;
    }
  }

  /* send a command to all BS to remove this slice */
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    const std::string s = std::to_string(slice_id);
    const std::string p = "{\"dl\":[{id:" + s
        + ",percentage:0}],\"ul\":[{id:" + s + ",percentage:0}]}";
    if (!remove_slice(bs_id, p, error_reason)) return false;
  }

  /* remove all IMSIs from the UE-slice association list for this slice */
  remove_ue_vnetwork(slice_id);
  return true;
}


bool flexran::app::management::rrm_management::parse_imsi_list(
    const std::string& list, std::vector<uint64_t>& imsis, std::string& error_reason)
{
  /* parses a in the form: [imsi1,imsi2,imsi3] */
  int i = 0;

  try {
    while (std::isspace(list.at(i))) ++i;
    if (list.at(i) != '[') {
      error_reason = "expected '[' at position " + std::to_string(i);
      goto error;
    }
    ++i;
    while (true) {
      while (std::isspace(list.at(i))) ++i;
      if (list.at(i) == ']') break;
      try {
        imsis.push_back(std::stoul(list.substr(i)));
      } catch (const std::invalid_argument& e) {
        error_reason = "could not convert number at position " + std::to_string(i);
        goto error;
      }
      while (std::isspace(list.at(i))) ++i;
      while (std::isdigit(list.at(i))) ++i;
      while (std::isspace(list.at(i))) ++i;
      if (list.at(i) == ']') break;
      if (list.at(i) != ',') {
        error_reason = "expected ',' at position " + std::to_string(i);
        goto error;
      }
      ++i;
    }
  } catch (const std::out_of_range& e) {
    error_reason = "unexpected list end";
    goto error;
  }

  return true;
error:
  imsis.clear();
  return false;
}

bool flexran::app::management::rrm_management::associate_ue_vnetwork(
    uint32_t slice_id, const std::string& policy, std::string& error_reason)
{
  int num_slices = 0;
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<flexran::rib::enb_rib_info> bs = rib_.get_bs(bs_id);
    const auto slices = bs->get_enb_config().cell_config(0).slice_config();
    for (const protocol::flex_dl_slice& dl : slices.dl()) {
      if (dl.id() == slice_id) {
        num_slices++;
        break;
      }
    }
    for (const protocol::flex_ul_slice& ul : slices.ul()) {
      if (ul.id() == slice_id) {
        num_slices++;
        break;
      }
    }
  }
  if (num_slices == 0) {
    error_reason = "no slices found";
    return false;
  }

  std::vector<uint64_t> imsis;
  if (!parse_imsi_list(policy, imsis, error_reason))
    return false;
  for (uint64_t imsi : imsis) {
    ue_slice_.emplace(imsi, slice_id);
    LOG4CXX_INFO(flog::app, "monitoring UE with IMSI " << imsi
        << " to be in slice " << slice_id);
  }

  /* change all UEs whose IMSI is in list "imsi" */
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    auto bs = rib_.get_bs(bs_id);
    for (const protocol::flex_ue_config& ue : bs->get_ue_configs().ue_config()) {
      if (!ue.has_imsi()) continue;
      const auto it = std::find(imsis.begin(), imsis.end(), ue.imsi());
      if (it != imsis.end())
        ue_add_update(bs_id, ue.rnti());
    }
  }
  return true;
}

int flexran::app::management::rrm_management::remove_ue_vnetwork(
    const std::string& p, std::string& error_reason)
{
  std::vector<uint64_t> imsis;
  if (!parse_imsi_list(p, imsis, error_reason))
    return -1;

  int r = 0;
  while (!imsis.empty()) {
    const auto imsi_it = imsis.begin();
    const auto it = ue_slice_.find(*imsi_it);
    if (it != ue_slice_.end()) {
      LOG4CXX_INFO(flog::app, "removing IMSI-slice association "
          << it->first << " <> " << it->second);
      ue_slice_.erase(it);
      r++;
    }
    imsis.erase(imsi_it);
  }
  return r;
}

int flexran::app::management::rrm_management::remove_ue_vnetwork(
    uint32_t slice_id)
{
  int r = 0;
  for (auto it = ue_slice_.begin(); it != ue_slice_.end(); ) {
    if (it->second == slice_id) {
      LOG4CXX_INFO(flog::app, "removing IMSI-slice association "
          << it->first << " <> " << it->second);
      it = ue_slice_.erase(it); // returns the reference to the next element
      r++;
    } else {
      it++;
    }
  }
  return r;
}

void flexran::app::management::rrm_management::ue_add_update(uint64_t bs_id,
    flexran::rib::rnti_t rnti)
{
  /* check given UE: if it is in ue_slice_ but not in the slice it is supposed
   * to be, we change its association */
  std::shared_ptr<flexran::rib::enb_rib_info> bs = rib_.get_bs(bs_id);
  if (!bs) return;

  /* find UE and verify it has IMSI & slice IDs */
  const auto ue_it = std::find_if(
      bs->get_ue_configs().ue_config().begin(),
      bs->get_ue_configs().ue_config().end(),
      [rnti] (const protocol::flex_ue_config& c) { return c.rnti() == rnti; }
  );
  if (ue_it == bs->get_ue_configs().ue_config().end()) return;
  if (!ue_it->has_imsi()) return;
  if (!ue_it->has_dl_slice_id()) return;
  if (!ue_it->has_ul_slice_id()) return;

  const auto it = ue_slice_.find(ue_it->imsi());
  if (it == ue_slice_.end()) return;
  if (!bs->has_dl_slice(it->second) || !bs->has_ul_slice(it->second)) {
    LOG4CXX_ERROR(flog::app, "no such slice " << it->second
        << " for BS " << bs_id);
      return;
  }
  /* if current and desired slice IDs don't match, change it */
  if (ue_it->dl_slice_id() != it->second || ue_it->ul_slice_id() != it->second) {
    /* TODO this could be done manually and would be faster */
    const std::string p = "{\"ueConfig\":[{\"imsi\":"
        + std::to_string(ue_it->imsi()) + ",\"dlSliceId\":"
        + std::to_string(it->second) + ",\"ulSliceId\":"
        + std::to_string(it->second) + "}]}";
    std::string e;
    if (!change_ue_slice_association(bs_id, p, e)) {
      LOG4CXX_ERROR(flog::app, "error: " + e);
    }
  }
}

void flexran::app::management::rrm_management::reconfigure_agent_string(
   uint64_t bs_id, std::string policy)
{
  protocol::flexran_message config_message;
  // Create control delegation message header
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);

  protocol::flex_agent_reconfiguration *agent_reconfiguration_msg(new protocol::flex_agent_reconfiguration);
  agent_reconfiguration_msg->set_allocated_header(config_header);

  agent_reconfiguration_msg->set_policy(policy);

  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_agent_reconfiguration_msg(agent_reconfiguration_msg);
  req_manager_.send_message(bs_id, config_message);
}

bool flexran::app::management::rrm_management::apply_slice_config_policy(
    uint64_t bs_id, const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_bs(bs_id)) {
    error_reason = "BS does not exist";
    LOG4CXX_ERROR(flog::app, "BS " << bs_id << " does not exist");
    return false;
  }

  protocol::flex_slice_config slice_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &slice_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf slice_config message:" << ret.ToString());
    return false;
  }

  // enforce every DL/UL slice has an ID and well formed parameters
  for (int i = 0; i < slice_config.dl_size(); i++) {
    if (!verify_dl_slice_config(slice_config.dl(i), error_reason)) {
      error_reason += " in DL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }
  for (int i = 0; i < slice_config.ul_size(); i++) {
    if (!verify_ul_slice_config(slice_config.ul(i), error_reason)) {
      error_reason += " in UL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  // enforce that the sum percentage is equal or below 100 percent
  if (!verify_global_slice_percentage(bs_id, slice_config, error_reason)) {
    LOG4CXX_ERROR(flog::app, error_reason);
    return false;
  }

  // no UL slice is allowed to have the same firstRb as any other (in fact,
  // together with the percentage value it is computed that they don't
  // overlap). Therefore, if we have one new slice without a firstRb value, try
  // to add firstRb by checking a "free" region. This is to keep the short
  // version of the slice configuration end point working and we therefore
  // check that this slice carries nothing except an ID */
  if (slice_config.ul_size() == 1
      && slice_config.ul(0).id() != 0
      && !slice_config.ul(0).has_label()
      && !slice_config.ul(0).has_percentage()
      && !slice_config.ul(0).has_isolation()
      && !slice_config.ul(0).has_priority()
      && !slice_config.ul(0).has_first_rb()
      && !slice_config.ul(0).has_maxmcs()
      && slice_config.ul(0).sorting_size() == 0
      && !slice_config.ul(0).has_accounting()
      && !slice_config.ul(0).has_scheduler_name()
      && try_add_first_rb(bs_id, *slice_config.mutable_ul(0)))
    LOG4CXX_WARN(flog::app, "no firstRb value detected, added "
        << slice_config.ul(0).first_rb() << " so that it does not clash in "
        << "the BS. You can override this by specifying a firstRb value.");

  protocol::flex_cell_config cell_config;
  cell_config.mutable_slice_config()->CopyFrom(slice_config);
  push_cell_config_reconfiguration(bs_id, cell_config);
  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(slice_config, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent new configuration to BS " << bs_id
      << ":\n" << pol_corrected);

  return true;
}

bool flexran::app::management::rrm_management::remove_slice(uint64_t bs_id,
    const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_bs(bs_id)) {
    error_reason = "BS does not exist";
    LOG4CXX_ERROR(flog::app, "BS " << bs_id << " does not exist");
    return false;
  }

  protocol::flex_slice_config slice_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &slice_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf slice_config message:" << ret.ToString());
    return false;
  }

  // enforce every DL/UL slice has an ID and well formed parameters
  for (int i = 0; i < slice_config.dl_size(); i++) {
    if (!verify_dl_slice_removal(slice_config.dl(i), error_reason)) {
      error_reason += " in DL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (!rib_.get_bs(bs_id)->has_dl_slice(slice_config.dl(i).id())) {
      error_reason = "DL slice " + std::to_string(slice_config.dl(i).id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }
  for (int i = 0; i < slice_config.ul_size(); i++) {
    if (!verify_ul_slice_removal(slice_config.ul(i), error_reason)) {
      error_reason += " in UL slice configuration at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (!rib_.get_bs(bs_id)->has_ul_slice(slice_config.ul(i).id())) {
      error_reason = "UL slice " + std::to_string(slice_config.ul(i).id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  protocol::flex_cell_config cell_config;
  cell_config.mutable_slice_config()->CopyFrom(slice_config);
  push_cell_config_reconfiguration(bs_id, cell_config);
  LOG4CXX_INFO(flog::app, "sent remove slice command to BS " << bs_id
      << ":\n" << policy << "\n");

  return true;
}

bool flexran::app::management::rrm_management::change_ue_slice_association(
    uint64_t bs_id, const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_bs(bs_id)) {
    error_reason = "BS does not exist";
    LOG4CXX_ERROR(flog::app, "BS " << bs_id << " does not exist");
    return false;
  }

  protocol::flex_ue_config_reply ue_config_reply;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &ue_config_reply,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf ue_config_reply message:" << ret.ToString());
    return false;
  }

  // enforce UE configaration
  if (ue_config_reply.ue_config_size() == 0) {
    error_reason = "Missing UE configuration";
    LOG4CXX_ERROR(flog::app,
        "the ue_config_reply message must contain a UE configuration");
    return false;
  }
  // enforce UE configuration has both RNTI and UL or DL slice ID
  for (int i = 0; i < ue_config_reply.ue_config_size(); i++) {
    if (!verify_ue_slice_assoc_msg(ue_config_reply.ue_config(i), error_reason)) {
      error_reason += " in UE-slice association at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (ue_config_reply.ue_config(i).has_dl_slice_id()
        && !rib_.get_bs(bs_id)->has_dl_slice(ue_config_reply.ue_config(i).dl_slice_id())) {
      error_reason = "DL slice "
          + std::to_string(ue_config_reply.ue_config(i).dl_slice_id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
    if (ue_config_reply.ue_config(i).has_ul_slice_id()
        && !rib_.get_bs(bs_id)->has_ul_slice(ue_config_reply.ue_config(i).ul_slice_id())) {
      error_reason = "UL slice "
          + std::to_string(ue_config_reply.ue_config(i).ul_slice_id())
          + " does not exist";
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  for (int i = 0; i < ue_config_reply.ue_config_size(); i++) {
    if (!verify_rnti_imsi(bs_id, ue_config_reply.mutable_ue_config(i), error_reason)) {
      error_reason += " in UE-slice association at index " + std::to_string(i);
      LOG4CXX_ERROR(flog::app, error_reason);
      return false;
    }
  }

  push_ue_config_reconfiguration(bs_id, ue_config_reply);
  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(ue_config_reply, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent new UE configuration to BS "
      << bs_id << ":\n" << pol_corrected);

  return true;
}

bool flexran::app::management::rrm_management::apply_cell_config_policy(
    uint64_t bs_id, const std::string& policy, std::string& error_reason)
{
  if (!rib_.get_bs(bs_id)) {
    error_reason = "BS does not exist";
    LOG4CXX_ERROR(flog::app, "BS " << bs_id << " does not exist");
    return false;
  }

  protocol::flex_cell_config cell_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &cell_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    error_reason = "ProtoBuf parser error";
    LOG4CXX_ERROR(flog::app,
        "error while parsing ProtoBuf ue_config_reply message:" << ret.ToString());
    return false;
  }

  if (!verify_cell_config_for_restart(cell_config, error_reason)) {
    LOG4CXX_ERROR(flog::app, error_reason);
    return false;
  }

  push_cell_config_reconfiguration(bs_id, cell_config);
  LOG4CXX_INFO(flog::app, "sent new cell configuration to BS " << bs_id
      << ":\n" << policy << "\n");

  return true;
}

void flexran::app::management::rrm_management::push_cell_config_reconfiguration(
    uint64_t bs_id, const protocol::flex_cell_config& cell_config)
{
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);

  protocol::flex_enb_config_reply *enb_config_msg(new protocol::flex_enb_config_reply);
  enb_config_msg->add_cell_config();
  enb_config_msg->mutable_cell_config(0)->CopyFrom(cell_config);
  enb_config_msg->set_allocated_header(config_header);

  protocol::flexran_message config_message;
  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_enb_config_reply_msg(enb_config_msg);
  req_manager_.send_message(bs_id, config_message);
}

void flexran::app::management::rrm_management::push_ue_config_reconfiguration(
    uint64_t bs_id, const protocol::flex_ue_config_reply& ue_config)
{
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);

  protocol::flex_ue_config_reply *ue_config_msg(new protocol::flex_ue_config_reply);
  ue_config_msg->CopyFrom(ue_config);
  ue_config_msg->set_allocated_header(config_header);

  protocol::flexran_message config_message;
  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_ue_config_reply_msg(ue_config_msg);
  req_manager_.send_message(bs_id, config_message);
}

bool flexran::app::management::rrm_management::verify_dl_slice_config(
    const protocol::flex_dl_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [0,255]";
    return false;
  }
  /* label is enum */
  if (s.has_percentage() && (s.percentage() < 1 || s.percentage() > 100)) {
    error_message = "DL percentage must be within [1,100]";
    return false;
  }
  /* isolation can only be true or false */
  if (s.has_priority() && s.priority() > 20) {
    error_message = "priority must be within [0,20]";
    return false;
  }
  if (s.has_position_low() && s.position_low() > 25) {
    error_message = "position_low must be within [0,25] (RBG)";
    return false;
  }
  if (s.has_position_high() && s.position_high() > 25) {
    error_message = "position_high must be within [0,25] (RBG)";
    return false;
  }
  if (s.has_position_low() && s.has_position_high()
      && s.position_low() >= s.position_high()) {
    error_message = "position_low must be smaller than position_high";
    return false;
  }
  if (s.has_maxmcs() && s.maxmcs() > 28) {
    error_message = "DL maxmcs must be within [0,28]";
    return false;
  }
  /* sorting is enum */
  /* accounting is enum */
  if (s.has_scheduler_name()) {
    error_message = "setting another scheduler is not supported";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_dl_slice_removal(
    const protocol::flex_dl_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [1,255]";
    return false;
  }
  if (s.id() == 0) {
    error_message = "DL Slice 0 can not be deleted";
    return false;
  }
  if (!s.has_percentage() || s.percentage() != 0) {
    error_message = "Slice removal requires percentage to be set to 0";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_ul_slice_config(
    const protocol::flex_ul_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [0,255]";
    return false;
  }
  /* label is enum */
  if (s.has_percentage() && (s.percentage() < 1 || s.percentage() > 100)) {
    error_message = "percentage must be within [1,100]";
    return false;
  }
  /* isolation can only be true or false */
  if (s.has_priority()) {
    error_message = "slice priority is not supported";
    return false;
  }
  if (s.has_first_rb() && s.first_rb() > 99) {
    error_message = "first_rb must be within [0,99] (RB)";
    return false;
  }
  /*if (s.has_length_rb()
      && (s.length_rb() < 1 || s.length_rb() > 100)) {
    error_message = "length_rb must be within [1,100] (RB)";
    return false;
  }
  if (s.has_length_rb() && s.has_first_rb() && s.length_rb() + s.first_rb() > 100) {
    error_message = "length_rb must be within [1,100-first_rb] (RB)";
    return false;
  }*/
  if (s.has_maxmcs() && s.maxmcs() > 20) {
    error_message = "UL maxmcs must be within [0,20]";
    return false;
  }
  /* sorting is enum */
  /* accounting is enum */
  if (s.has_scheduler_name()) {
    error_message = "setting another scheduler is not supported";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_ul_slice_removal(
    const protocol::flex_ul_slice& s, std::string& error_message)
{
  if (!s.has_id()) {
    error_message = "Missing slice ID";
    return false;
  }
  if (s.id() > 255) {
    error_message = "Slice ID must be within [1,255]";
    return false;
  }
  if (s.id() == 0) {
    error_message = "UL Slice 0 can not be deleted";
    return false;
  }
  if (!s.has_percentage() || s.percentage() != 0) {
    error_message = "Slice removal requires percentage to be set to 0";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_global_slice_percentage(
    uint64_t bs_id, const protocol::flex_slice_config& c, std::string& error_message)
{
  auto h = rib_.get_bs(bs_id);
  if (h == nullptr) {
    error_message = "no such BS";
    return false;
  }
  const protocol::flex_slice_config& ex = h->get_enb_config().cell_config(0).slice_config();
  return verify_global_dl_slice_percentage(ex, c, error_message)
      && verify_global_ul_slice_percentage(ex, c, error_message);
}

bool flexran::app::management::rrm_management::verify_global_dl_slice_percentage(
    const protocol::flex_slice_config& existing,
    const protocol::flex_slice_config& update, std::string& error_message)
{
  std::map<int, int> slice_pct;
  for (int i = 0; i < existing.dl_size(); i++)
    slice_pct[existing.dl(i).id()] = existing.dl(i).percentage();
  for (int i = 0; i < update.dl_size(); i++)
    // the BS will copy the values from slice 0 if not specified, so do we
    slice_pct[update.dl(i).id()] = update.dl(i).has_percentage() ?
        update.dl(i).percentage() : slice_pct[0];
  int sum = 0;
  for (const auto &p: slice_pct)
    sum += p.second;
  if (sum > 100) {
    error_message = "resulting DL slice sum percentage exceeds 100";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_global_ul_slice_percentage(
    const protocol::flex_slice_config& existing,
    const protocol::flex_slice_config& update, std::string& error_message)
{
  std::map<int, int> slice_pct;
  for (int i = 0; i < existing.ul_size(); i++)
    slice_pct[existing.ul(i).id()] = existing.ul(i).percentage();
  for (int i = 0; i < update.ul_size(); i++)
    // the BS will copy the values from slice 0 if not specified, so do we
    slice_pct[update.ul(i).id()] = update.ul(i).has_percentage() ?
        update.ul(i).percentage() : slice_pct[0];
  int sum = 0;
  for (const auto &p: slice_pct)
    sum += p.second;
  if (sum > 100) {
    error_message = "resulting UL slice sum percentage exceeds 100";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_ue_slice_assoc_msg(
    const protocol::flex_ue_config& c, std::string& error_message)
{

  if (!c.has_rnti() && !c.has_imsi()) {
    error_message = "Missing RNTI or IMSI";
    return false;
  }
  if (!c.has_dl_slice_id() && !c.has_ul_slice_id()) {
    error_message = "No DL or UL slice ID";
    return false;
  }
  return true;
}

bool flexran::app::management::rrm_management::verify_rnti_imsi(
    uint64_t bs_id, protocol::flex_ue_config *c, std::string& error_message)
{
  // if RNTI present but there is no corresponding UE, abort
  if (c->has_rnti() && !rib_.get_bs(bs_id)->get_ue_mac_info(c->rnti())) {
    error_message = "a UE with RNTI" + std::to_string(c->rnti()) + " does not exist";
    return false;
  }

  // there is an RNTI, the corresponding UE exists and no IMSI that could
  // contradict -> can leave
  if (!c->has_imsi())
    return true;

  uint64_t imsi = c->imsi();
  flexran::rib::rnti_t rnti;
  if (!rib_.get_bs(bs_id)->get_rnti(imsi, rnti)) {
    error_message = "IMSI " + std::to_string(imsi) + " is not present";
    return false;
  }

  if (rnti == 0) {
      error_message = "found invalid RNTI 0 for IMSI " + std::to_string(imsi);
      return false;
  }

  if (c->has_rnti() && c->rnti() != rnti) {
    error_message = "RNTI-IMSI mismatch";
    return false;
  }

  c->set_rnti(rnti);
  return true;
}

bool flexran::app::management::rrm_management::verify_cell_config_for_restart(
    const protocol::flex_cell_config& c, std::string& error_message)
{
  if (c.has_phy_cell_id()) {
    error_message = "setting phy_cell_id not supported";
    return false;
  }
  if (c.has_pusch_hopping_offset()) {
    error_message = "setting pusch_hopping_offset not supported";
    return false;
  }
  if (c.has_hopping_mode()) {
    error_message = "setting hopping_mode not supported";
    return false;
  }
  if (c.has_n_sb()) {
    error_message = "setting n_sb not supported";
    return false;
  }
  if (c.has_phich_resource()) {
    error_message = "setting phich_resource not supported";
    return false;
  }
  if (c.has_phich_duration()) {
    error_message = "setting phich_durationnot supported";
    return false;
  }
  if (c.has_init_nr_pdcch_ofdm_sym()) {
    error_message = "setting init_nr_pdcch_ofdm_sym not supported";
    return false;
  }
  if (c.has_si_config()) {
    error_message = "setting si_config not supported";
    return false;
  }
  if (c.has_ul_cyclic_prefix_length()) {
    error_message = "setting ul_cyclic_prefix_length not supported";
    return false;
  }
  if (c.has_dl_cyclic_prefix_length()) {
    error_message = "setting dl_cyclic_prefix_length not supported";
    return false;
  }
  if (c.has_antenna_ports_count()) {
    error_message = "setting antenna_ports_count not supported";
    return false;
  }
  if (c.has_duplex_mode()) {
    error_message = "setting duplex_mode not supported";
    return false;
  }
  if (c.has_subframe_assignment()) {
    error_message = "setting subframe_assignment not supported";
    return false;
  }
  if (c.has_special_subframe_patterns()) {
    error_message = "setting special_subframe_patterns not supported";
    return false;
  }
  if (c.mbsfn_subframe_config_rfperiod_size() > 0) {
    error_message = "setting mbsfn_subframe_config_rfperiod not supported";
    return false;
  }
  if (c.mbsfn_subframe_config_rfoffset_size() > 0) {
    error_message = "setting mbsfn_subframe_config_rfoffset not supported";
    return false;
  }
  if (c.mbsfn_subframe_config_sfalloc_size() > 0) {
    error_message = "setting mbsfn_subframe_config_sfalloc not supported";
    return false;
  }
  if (c.has_prach_config_index()) {
    error_message = "setting prach_config_index not supported";
    return false;
  }
  if (c.has_prach_freq_offset()) {
    error_message = "setting prach_freq_offset not supported";
    return false;
  }
  if (c.has_ra_response_window_size()) {
    error_message = "setting ra_response_window_size not supported";
    return false;
  }
  if (c.has_mac_contention_resolution_timer()) {
    error_message = "setting mac_contention_resolution_timer not supported";
    return false;
  }
  if (c.has_max_harq_msg3tx()) {
    error_message = "setting max_harq_msg3tx not supported";
    return false;
  }
  if (c.has_n1pucch_an()) {
    error_message = "setting n1pucch_an not supported";
    return false;
  }
  if (c.has_deltapucch_shift()) {
    error_message = "setting deltapucch_shift not supported";
    return false;
  }
  if (c.has_nrb_cqi()) {
    error_message = "setting nrb_cqi not supported";
    return false;
  }
  if (c.has_srs_subframe_config()) {
    error_message = "setting srs_subframe_config not supported";
    return false;
  }
  if (c.has_srs_bw_config()) {
    error_message = "setting srs_bw_config not supported";
    return false;
  }
  if (c.has_srs_mac_up_pts()) {
    error_message = "setting srs_mac_up_pts not supported";
    return false;
  }
  if (c.has_enable_64qam()) {
    error_message = "setting enable_64qam not supported";
    return false;
  }
  if (c.has_carrier_index()) {
    error_message = "setting not supported yet, defaults to 0";
    return false;
  }
  if (c.has_slice_config()) {
    error_message = "setting slice_config not supported, use another end point";
    return false;
  }
  /* if no band is given, we simply assume band 7 */
  if (!c.has_eutra_band()) {
    error_message = "eutra_band must be present";
    return false;
  }
  if (!c.has_dl_freq() || !c.has_ul_freq()) {
    error_message = "both dl_freq and ul_freq must be present";
    return false;
  }
  if (!c.has_dl_bandwidth() || !c.has_ul_bandwidth()) {
    error_message = "both dl_bandwidth and ul_bandwidth must be present";
    return false;
  }
  if (c.dl_bandwidth() != c.ul_bandwidth()) {
    error_message = "dl_bandwidth and ul_bandwidth must be the same (6, 15, 25, 50, 100)";
    return false;
  }
  if (c.plmn_id_size() > 0) {
    error_message = "setting PLMNs not supported";
    return false;
  }
  if (!check_eutra_bandwidth(c.dl_bandwidth(), error_message))
    return false;
  // checking function tests against Hz, but ul_freq/dl_freq are in MHz!
  if (!check_eutra_band(c.eutra_band(), c.ul_freq() * 1000000, c.dl_freq() * 1000000, error_message, c.dl_bandwidth(), true))
    return false;

  return true;
}

uint64_t flexran::app::management::rrm_management::parse_enb_agent_id(
    const std::string& enb_agent_id_s) const
{
  return rib_.parse_enb_agent_id(enb_agent_id_s);
}

uint64_t flexran::app::management::rrm_management::get_last_bs() const
{
  if (rib_.get_available_base_stations().empty())
    return 0;

  return *std::prev(rib_.get_available_base_stations().end());
}

bool flexran::app::management::rrm_management::parse_rnti_imsi(
    uint64_t bs_id, const std::string& rnti_imsi_s,
    flexran::rib::rnti_t& rnti) const
{
  return rib_.get_bs(bs_id)->parse_rnti_imsi(rnti_imsi_s, rnti);
}

bool flexran::app::management::rrm_management::try_add_first_rb(
    uint64_t bs_id, protocol::flex_ul_slice& slice)
{
  // this function is dumb: it simply assumes that all existing slices are
  // adjacent and only one is added. Therefore, it picks the highest first_Rb,
  // adds N_RB * percentage and adds this to the existing slice.
  auto h = rib_.get_bs(bs_id);
  if (h == nullptr) return false;
  const protocol::flex_slice_config& ex = h->get_enb_config().cell_config(0).slice_config();
  if (ex.ul_size() < 1) return false;
  const int N_RB = h->get_enb_config().cell_config(0).ul_bandwidth();
  const int pct = ex.ul(0).percentage();
  const int first_rb = ex.ul(ex.ul_size() - 1).first_rb();
  slice.set_first_rb(first_rb + pct * N_RB / 100);
  return true;
}

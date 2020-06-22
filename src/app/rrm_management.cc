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

void flexran::app::management::rrm_management::apply_slice_config_policy(
    const std::string& bs_, const std::string& policy)
{
  uint64_t bs_id = rib_.parse_bs_id(bs_);
  if (bs_id == 0)
    throw std::invalid_argument("BS " + bs_ + " does not exist");

  protocol::flex_slice_config slice_config;
  auto ret = google::protobuf::util::JsonStringToMessage(policy, &slice_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    LOG4CXX_ERROR(flog::app, "error while parsing ProtoBuf message:" << ret.ToString());
    throw std::invalid_argument("Protobuf parser error");
  }

  const auto bs = rib_.get_bs(bs_id);
  const auto& current = bs->get_enb_config().cell_config(0).slice_config();

  if (!slice_config.has_dl()) {
    auto *dl(new protocol::flex_slice_dl_ul_config);
    slice_config.set_allocated_dl(dl);
  }
  protocol::flex_slice_dl_ul_config *dl = slice_config.mutable_dl();
  if (!dl->has_algorithm())
    dl->set_algorithm(current.dl().algorithm());
  if (dl->algorithm() == protocol::flex_slice_algorithm::Static) {
    if (dl->slices_size() == 0)
      *dl = dl_transform_to_static_slice_configuration(
          bs, bs->get_enb_config().cell_config(0).slice_config());
    else
      verify_static_slice_configuration(*dl, current.dl());
  }
  if (dl->algorithm() == protocol::flex_slice_algorithm::NVS) {
    if (dl->slices_size() == 0)
      *dl = dl_transform_to_nvs_slice_configuration(
          bs, bs->get_enb_config().cell_config(0).slice_config());
    else
      verify_nvs_slice_configuration(*dl, bs, current.dl());
  }

  if (!slice_config.has_ul()) {
    auto *ul(new protocol::flex_slice_dl_ul_config);
    slice_config.set_allocated_ul(ul);
  }
  protocol::flex_slice_dl_ul_config *ul = slice_config.mutable_ul();
  if (!ul->has_algorithm())
    ul->set_algorithm(current.ul().algorithm());
  if (ul->algorithm() == protocol::flex_slice_algorithm::Static)
    verify_static_slice_configuration(*ul, current.ul());
  if (ul->algorithm() == protocol::flex_slice_algorithm::NVS)
    throw std::invalid_argument("NVS not handled in uplink");

  if (dl->algorithm() == current.dl().algorithm()
      && ul->algorithm() == current.ul().algorithm()
      && dl->slices_size() == 0 && ul->slices_size() == 0
      && !dl->has_scheduler() && !ul->has_scheduler())
    return;

  if ((dl->algorithm() == protocol::flex_slice_algorithm::None && dl->slices_size() > 0)
      || (ul->algorithm() == protocol::flex_slice_algorithm::None && ul->slices_size() > 0))
    throw std::invalid_argument("no slice algorithm, but slices present");

  bool algo_change = current.dl().algorithm() != dl->algorithm()
                     || current.ul().algorithm() != ul->algorithm();

  protocol::flex_cell_config cell_config;
  cell_config.mutable_slice_config()->CopyFrom(slice_config);
  push_cell_config_reconfiguration(bs_id, cell_config);
  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(slice_config, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent new configuration to BS " << bs_id
      << ":\n" << pol_corrected);

  if (!algo_change || bs->get_ue_configs().ue_config_size() == 0)
    return;
  LOG4CXX_INFO(flog::app, "ue_config_size() " << bs->get_ue_configs().ue_config_size());
  /* if there is an algorithm change, try to preserve the UE-slice association:
   * Go through all UEs and check whether the slice exists, then associate */
  protocol::flex_ue_config_reply ue_config_reply;
  for (const auto& ue : bs->get_ue_configs().ue_config()) {
    uint32_t did = ue.has_dl_slice_id() ? ue.dl_slice_id() : 0;
    uint32_t uid = ue.has_ul_slice_id() ? ue.ul_slice_id() : 0;
    if (!std::any_of(dl->slices().begin(), dl->slices().end(),
          [did] (const protocol::flex_slice& s) { return s.id() == did; }))
      did = 0;
    if (!std::any_of(ul->slices().begin(), ul->slices().end(),
          [uid] (const protocol::flex_slice& s) { return s.id() == uid; }))
      uid = 0;
    if (did != 0 || uid != 0) { // only send if it makes sense
      auto *c = ue_config_reply.add_ue_config();
      c->set_rnti(ue.rnti());
      c->set_dl_slice_id(did);
      c->set_ul_slice_id(uid);
    }
  }
  push_ue_config_reconfiguration(bs_id, ue_config_reply);
  std::string ue_policy;
  google::protobuf::util::MessageToJsonString(ue_config_reply, &ue_policy, opt);
  LOG4CXX_INFO(flog::app, "sent new UE configuration to BS "
      << bs_id << ":\n" << ue_policy);
}

void flexran::app::management::rrm_management::remove_slice(
    const std::string& bs, const std::string& policy)
{
  uint64_t bs_id = rib_.parse_bs_id(bs);
  if (bs_id == 0)
    throw std::invalid_argument("BS " + bs + " does not exist");

  protocol::flex_slice_config slice_config;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &slice_config,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    LOG4CXX_ERROR(flog::app, "error while parsing ProtoBuf message:" << ret.ToString());
    throw std::invalid_argument("Protobuf parser error");
  }

  slice_config.mutable_dl()->clear_algorithm();
  slice_config.mutable_ul()->clear_algorithm();
  auto f = [](const protocol::flex_slice& s) { return s.has_id(); };
  if (!std::all_of(slice_config.dl().slices().begin(), slice_config.dl().slices().end(), f)
      || !std::all_of(slice_config.ul().slices().begin(), slice_config.ul().slices().end(), f))
    throw std::invalid_argument("all slices must have an ID and no params");
  for (auto& s: *slice_config.mutable_dl()->mutable_slices())
    s.clear_static_();
  for (auto& s: *slice_config.mutable_ul()->mutable_slices())
    s.clear_static_();

  protocol::flex_cell_config cell_config;
  cell_config.mutable_slice_config()->CopyFrom(slice_config);
  push_cell_config_reconfiguration(bs_id, cell_config);

  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(slice_config, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent remove slice command to BS " << bs_id
      << ":\n" << pol_corrected << "\n");
}

void flexran::app::management::rrm_management::change_ue_slice_association(
    const std::string& bs, const std::string& policy)
{
  uint64_t bs_id = rib_.parse_bs_id(bs);
  if (bs_id == 0)
    throw std::invalid_argument("BS " + bs + " does not exist");

  protocol::flex_ue_config_reply ue_config_reply;
  google::protobuf::util::Status ret;
  ret = google::protobuf::util::JsonStringToMessage(policy, &ue_config_reply,
      google::protobuf::util::JsonParseOptions());
  if (ret != google::protobuf::util::Status::OK) {
    LOG4CXX_ERROR(flog::app, "error while parsing ProtoBuf message:" << ret.ToString());
    throw std::invalid_argument("Protobuf parser error");
  }

  // enforce UE configaration
  if (ue_config_reply.ue_config_size() == 0)
    throw std::invalid_argument("missing UE configuration");
  // enforce UE configuration has both RNTI and UL or DL slice ID
  for (int i = 0; i < ue_config_reply.ue_config_size(); i++) {
    std::string error_reason;
    if (!verify_ue_slice_assoc_msg(ue_config_reply.ue_config(i), error_reason)) {
      error_reason += " in UE-slice association at index " + std::to_string(i);
      throw std::invalid_argument(error_reason);
    }
    if (ue_config_reply.ue_config(i).has_dl_slice_id()
        && !rib_.get_bs(bs_id)->has_dl_slice(ue_config_reply.ue_config(i).dl_slice_id())) {
      error_reason = "DL slice "
          + std::to_string(ue_config_reply.ue_config(i).dl_slice_id())
          + " does not exist";
      throw std::invalid_argument(error_reason);
    }
    if (ue_config_reply.ue_config(i).has_ul_slice_id()
        && !rib_.get_bs(bs_id)->has_ul_slice(ue_config_reply.ue_config(i).ul_slice_id())) {
      error_reason = "UL slice "
          + std::to_string(ue_config_reply.ue_config(i).ul_slice_id())
          + " does not exist";
      throw std::invalid_argument(error_reason);
    }
  }

  for (int i = 0; i < ue_config_reply.ue_config_size(); i++) {
    std::string error_reason;
    if (!verify_rnti_imsi(bs_id, ue_config_reply.mutable_ue_config(i), error_reason)) {
      error_reason += " in UE-slice association at index " + std::to_string(i);
      throw std::invalid_argument(error_reason);
    }
  }

  push_ue_config_reconfiguration(bs_id, ue_config_reply);
  std::string pol_corrected;
  google::protobuf::util::JsonPrintOptions opt;
  opt.add_whitespace = true;
  google::protobuf::util::MessageToJsonString(ue_config_reply, &pol_corrected, opt);
  LOG4CXX_INFO(flog::app, "sent new UE configuration to BS "
      << bs_id << ":\n" << pol_corrected);
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

void flexran::app::management::rrm_management::
    verify_static_slice_configuration(
      const protocol::flex_slice_dl_ul_config& nc,
      const protocol::flex_slice_dl_ul_config& exist)
{
  if (nc.has_scheduler())
    throw std::invalid_argument("cannot have a single scheduler");

  /* do all slices have an ID and (correct) parameters? */
  auto f = [](const protocol::flex_slice& s) {
    return s.has_id() && (s.has_label() || s.has_scheduler() ||s.has_static_());
  };
  if (!std::all_of(nc.slices().begin(), nc.slices().end(), f))
    throw std::invalid_argument("all slices need to have an ID and parameters");

  /* checks that if slice does not exist, needs to have full parameters */
  auto p = [exist](const protocol::flex_slice& s) {
    return std::any_of(exist.slices().begin(), exist.slices().end(),
            [s] (const protocol::flex_slice& es) { return es.id() == s.id(); })
           || (s.has_static_() && s.static_().has_poslow() && s.static_().has_poshigh()); };
  if (!std::all_of(nc.slices().begin(), nc.slices().end(), p))
    throw std::invalid_argument("all new slices need to have complete parameters");

  int rbg[25] = {};
  for (auto &s: nc.slices()) {
    if (!s.has_static_())
      continue;
    for (unsigned int i = s.static_().poslow(); i <= s.static_().poshigh(); i++) {
      if (rbg[i])
        throw std::invalid_argument("overlapping slices at RBG " + std::to_string(i) + " for slice " + std::to_string(s.id()));
      rbg[i] = 1;
    }
  }
  for (auto &s: exist.slices()) {
    bool in_config = std::any_of(nc.slices().begin(), nc.slices().end(),
          [s] (const protocol::flex_slice &cs) { return cs.id() == s.id(); });
    if (in_config) /* if existing slices is reconfigured in config */
      continue;
    for (unsigned int i = s.static_().poslow(); i <= s.static_().poshigh(); i++) {
      if (rbg[i])
        throw std::invalid_argument("overlapping slices at RBG " + std::to_string(i) + " for existing slice " + std::to_string(s.id()));
      rbg[i] = 1;
    }
  }
}

protocol::flex_slice_dl_ul_config
flexran::app::management::rrm_management::dl_transform_to_static_slice_configuration(
        const std::shared_ptr<flexran::rib::enb_rib_info> bs,
        const protocol::flex_slice_config& c)
{
  const uint32_t bw = bs->get_enb_config().cell_config(0).dl_bandwidth();
  if (bw != 100 && bw != 50 && bw !=25)
    throw std::invalid_argument("cannot transform slice configuration for BW "
                                + std::to_string(bw));
  const uint32_t RBGsize = bw == 100 ? 4 : (bw == 50 ? 3 : 2);
  const uint32_t RBGs = (float) (bw + (bw % RBGsize)) / RBGsize;

  protocol::flex_slice_dl_ul_config dl;
  dl.set_algorithm(protocol::flex_slice_algorithm::Static);
  switch (c.dl().algorithm()) {
    case protocol::flex_slice_algorithm::None:
      return dl;
    case protocol::flex_slice_algorithm::Static:
      return c.dl();
    case protocol::flex_slice_algorithm::NVS: {
        uint32_t start = 0;
        for (const protocol::flex_slice& es: c.dl().slices()) {
          const auto& nvs_ = es.nvs();
          const float p = nvs_.has_pct_reserved() ?
              nvs_.pct_reserved()
              : nvs_.rate().mbps_required() / nvs_.rate().mbps_reference();
          const uint32_t len = std::round(p * RBGs);
          if (len == 0)
            throw std::invalid_argument("cannot approximate percentage "
                + std::to_string(p) + " in RBGs");

          protocol::flex_slice_static *static_(new protocol::flex_slice_static);
          static_->set_poslow(start);
          static_->set_poshigh(start + len - 1);
          auto *s = dl.add_slices();
          s->set_allocated_static_(static_);
          s->set_id(es.id());
          if (es.has_label())
            s->set_label(es.label());
          if (es.has_scheduler())
            s->set_scheduler(es.scheduler());
          start += len + 1;
        }
      }
      return dl;
    default:
      throw std::invalid_argument("transformation from unknown slice algorithm");
  }
}

void flexran::app::management::rrm_management::verify_nvs_slice_configuration(
      protocol::flex_slice_dl_ul_config& nc,
      const std::shared_ptr<flexran::rib::enb_rib_info> bs,
      const protocol::flex_slice_dl_ul_config& exist)
{
  const uint32_t bw = bs->get_enb_config().cell_config(0).dl_bandwidth();
  const float mrate = bw == 100 ? 70.0f : (bw == 50 ? 35.0f : 17.5f);

  auto f = [](const protocol::flex_slice& s) {
    return s.has_id() && (s.has_label() || s.has_scheduler() || s.has_nvs());
  };
  if (!std::all_of(nc.slices().begin(), nc.slices().end(), f))
    throw std::invalid_argument("all slices need to have an ID and parameters");
  for (protocol::flex_slice& s : *nc.mutable_slices()) {
    if (s.nvs().has_rate() && s.nvs().rate().has_mbps_required() && !s.nvs().rate().has_mbps_reference())
      s.mutable_nvs()->mutable_rate()->set_mbps_reference(mrate);
  }
  auto has_param = [](const protocol::flex_slice& s) {
    return s.nvs().has_pct_reserved()
      || (s.nvs().has_rate()
          && s.nvs().rate().has_mbps_required()
          && s.nvs().rate().has_mbps_reference()); };
  /* checks that if slice does not exist, needs to have full parameters */
  auto p = [exist,has_param](const protocol::flex_slice& s) {
    return std::any_of(exist.slices().begin(), exist.slices().end(),
            [s] (const protocol::flex_slice& es) { return es.id() == s.id(); })
           || has_param(s); };
  if (!std::all_of(nc.slices().begin(), nc.slices().end(), p))
    throw std::invalid_argument("all slices need to have complete parameters");

  float sum_req = 0.0f;
  for (auto &s: nc.slices()) {
    if (s.nvs().has_rate())
      sum_req += s.nvs().rate().mbps_required() / s.nvs().rate().mbps_reference();
    else if (s.nvs().has_pct_reserved())
      sum_req += s.nvs().pct_reserved();
  }
  for (auto &s: exist.slices()) {
    bool in_config = std::any_of(nc.slices().begin(), nc.slices().end(),
          [s] (const protocol::flex_slice &cs) { return cs.id() == s.id(); });
    if (in_config) /* if existing slices is reconfigured in config */
      continue;
    if (s.nvs().has_rate())
      sum_req += s.nvs().rate().mbps_required() / s.nvs().rate().mbps_reference();
    else if (s.nvs().has_pct_reserved())
      sum_req += s.nvs().pct_reserved();
  }
  if (sum_req > 1.0f)
    throw std::invalid_argument("sum of slice resources exceeds 1.0f");
}

protocol::flex_slice_dl_ul_config
flexran::app::management::rrm_management::dl_transform_to_nvs_slice_configuration(
    const std::shared_ptr<flexran::rib::enb_rib_info> bs,
    const protocol::flex_slice_config& c)
{
  const uint32_t bw = bs->get_enb_config().cell_config(0).dl_bandwidth();
  if (bw != 100 && bw != 50 && bw !=25)
    throw std::invalid_argument("cannot transform slice configuration for BW "
                                + std::to_string(bw));
  const uint32_t RBGsize = bw == 100 ? 4 : (bw == 50 ? 3 : 2);
  const uint32_t RBGs = (bw + (bw % RBGsize)) / RBGsize;

  protocol::flex_slice_dl_ul_config dl;
  dl.set_algorithm(protocol::flex_slice_algorithm::NVS);
  switch (c.dl().algorithm()) {
    case protocol::flex_slice_algorithm::None:
      return dl;
    case protocol::flex_slice_algorithm::Static: {
        for (const protocol::flex_slice& es: c.dl().slices()) {
          const float p = (float) (es.static_().poshigh() + 1 - es.static_().poslow()) / RBGs;
          LOG4CXX_INFO(flog::app, "H " << es.static_().poshigh() << " L " << es.static_().poslow() << " D "
              << es.static_().poshigh() + 1 - es.static_().poslow() << " P " << p << " RBGs " << RBGs);
          protocol::flex_slice_nvs *nvs_(new protocol::flex_slice_nvs);
          nvs_->set_pct_reserved(p);
          auto *s = dl.add_slices();
          s->set_allocated_nvs(nvs_);
          s->set_id(es.id());
          if (es.has_label())
            s->set_label(es.label());
          if (es.has_scheduler())
            s->set_scheduler(es.scheduler());
        }
      }
      return dl;
    case protocol::flex_slice_algorithm::NVS:
      return c.dl();
    default:
      throw std::invalid_argument("transformation from unknown slice algorithm");
  }
}

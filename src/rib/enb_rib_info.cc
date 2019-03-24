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

/*! \file    enb_rib_info.cc
 *  \brief   wrapper class for a cell's configuration
 *  \authors Xenofon Foukas, Navid Nikaein, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           robert.schmidt@eurecom.fr
 */

#include <iostream>
#include <algorithm>
#include <stdexcept>

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/reflection.h>

#include "enb_rib_info.h"
#include "flexran_log.h"



flexran::rib::enb_rib_info::enb_rib_info(uint64_t bs_id,
    const std::set<std::shared_ptr<agent_info>>& agents)
  : bs_id_(bs_id),
    agents_(agents)
{
  last_checked = st_clock::now();
  for (auto a: agents) {
    if (a->bs_id != bs_id_)
      throw std::runtime_error("invalid bs_id " + std::to_string(a->bs_id)
          + " from agent " + std::to_string(a->agent_id)
          + " for BS " + std::to_string(bs_id));
  }
  /* TODO check capabilities is complete */
}

void flexran::rib::enb_rib_info::update_eNB_config(
    const protocol::flex_enb_config_reply& enb_config_update)
{
  // we cannot simply call eNB_config_.MergeFrom as this would append repeated
  // fields (e.g. "PHY agent" has part of flex_cell_config, "RRC agent" has
  // one, leaving two flex_cell_configs instead of one unified), therefore we
  // do this independently for every flex_cell_config. In the following, we
  // also suppose that it is safe to replace(!) repeated fields within a
  // flex_cell_config, eg. mbsfn_subframe_config_rfperiod

  if (eNB_config_.cell_config_size() > 0
      && eNB_config_.cell_config_size() != enb_config_update.cell_config_size())
    LOG4CXX_WARN(flog::rib, __func__ << "(): differing numbers of cell_configs");

  eNB_config_mutex_.lock();
  if (eNB_config_.cell_config_size() == 0) { // saved config is empty
    eNB_config_.CopyFrom(enb_config_update);
  } else {
    const int n = std::min(eNB_config_.cell_config_size(), enb_config_update.cell_config_size());
    for (int i = 0; i < n; ++i) {
      protocol::flex_cell_config *dst = eNB_config_.mutable_cell_config(i);
      const protocol::flex_cell_config& src = enb_config_update.cell_config(i);
      clear_repeated_if_present(dst, src);
      /* the above should be recursively going down. ATM, delete complete slice
       * config if present */
      if (src.has_slice_config()) dst->clear_slice_config();
      dst->MergeFrom(src);
    }
    eNB_config_.mutable_s1ap()->CopyFrom(enb_config_update.s1ap());
  }
  eNB_config_mutex_.unlock();
  update_liveness();
}

void flexran::rib::enb_rib_info::update_UE_config(
    const protocol::flex_ue_config_reply& ue_config_update)
{
  // we cannot simply call ue_config_.MergeFrom as this would append repeated
  // fields (e.g. "MAC agent" has part of flex_ue_config, "RRC agent" has
  // one, leaving two flex_ue_configs instead of one unified), therefore we
  // do this independently for every flex_ue_config. In the following, we
  // also suppose that it is safe to replace(!) repeated fields within a
  // flex_ue_config, eg. mbsfn_subframe_config_rfperiod.


  if (ue_config_.ue_config_size() != ue_config_update.ue_config_size()) {
    LOG4CXX_WARN(flog::rib, __func__ << "(): BS " << bs_id_ << ": "
        << "number of UEs (" << ue_config_update.ue_config_size()
        << ") in update different than internal state ("
        << ue_config_.ue_config_size() << ")");
  }

  ue_config_mutex_.lock();
  for (const protocol::flex_ue_config& src : ue_config_update.ue_config()) {
    const rnti_t rnti = src.rnti();
    auto it = std::find_if(ue_config_.mutable_ue_config()->begin(),
                           ue_config_.mutable_ue_config()->end(),
        [rnti](protocol::flex_ue_config& c) { return c.rnti() == rnti; });
    if (it == ue_config_.mutable_ue_config()->end()) // this one does not exist
      continue;
    protocol::flex_ue_config *dst = &(*it);
    clear_repeated_if_present(dst, src);
    if (src.has_info())
      clear_repeated_if_present(dst->mutable_info(), src.info());
    dst->MergeFrom(src);
  }
  ue_config_mutex_.unlock();

  update_liveness();
}

void flexran::rib::enb_rib_info::update_UE_config(
    const protocol::flex_ue_state_change& ue_state_change)
{
  // releases itself when leaving scope
  std::lock_guard<std::mutex> lg_ue(ue_config_mutex_);
  std::lock_guard<std::mutex> lg_lc(lc_config_mutex_);
  const rnti_t rnti = ue_state_change.config().rnti();
  google::protobuf::RepeatedPtrField<protocol::flex_ue_config>::iterator it = ue_config_.mutable_ue_config()->begin();
  it = std::find_if(ue_config_.mutable_ue_config()->begin(), ue_config_.mutable_ue_config()->end(),
      [rnti] (const protocol::flex_ue_config& c) { return rnti == c.rnti(); }
  );

  switch (ue_state_change.type()) {
  case protocol::FLUESC_ACTIVATED:
    LOG4CXX_INFO(flog::rib, "BS " << bs_id_ << ": UE RNTI " << rnti << " activated");
    /* create new entry if not present, otherwise just update */
    if (it == ue_config_.mutable_ue_config()->end()) {
      protocol::flex_ue_config *c = ue_config_.add_ue_config();
      c->CopyFrom(ue_state_change.config());
      ue_mac_info_.emplace(rnti, std::make_shared<ue_mac_rib_info>(rnti));
    } else {
      /* dereference RepeatedPtrIterator, pass raw pointer */
      clear_repeated_if_present(&(*it), ue_state_change.config());
      it->MergeFrom(ue_state_change.config());
    }
    break;
  case protocol::FLUESC_DEACTIVATED:
    LOG4CXX_INFO(flog::rib, "BS " << bs_id_ << ": UE RNTI " << rnti << " deactivated");
    if (it != ue_config_.mutable_ue_config()->end()) {
      ue_config_.mutable_ue_config()->erase(it);
      ue_mac_info_.erase(rnti);
      auto lcit = std::find_if(lc_config_.lc_ue_config().cbegin(), lc_config_.lc_ue_config().cend(),
          [rnti] (const protocol::flex_lc_ue_config& c) { return rnti == c.rnti(); }
      );
      if (lcit != lc_config_.lc_ue_config().cend())
        lc_config_.mutable_lc_ue_config()->erase(lcit);
    }
    break;
  case protocol::FLUESC_UPDATED:
    LOG4CXX_INFO(flog::rib, "BS " << bs_id_ << ": UE RNTI " << rnti << " updated");
    if (it != ue_config_.mutable_ue_config()->end()) {
      clear_repeated_if_present(&(*it), ue_state_change.config());
      it->MergeFrom(ue_state_change.config());
    }
    break;
  default:
    LOG4CXX_WARN(flog::rib, "unhandled ue_state_change type " << ue_state_change.type()
        << " in " << __func__);
  }
}

void flexran::rib::enb_rib_info::update_LC_config(const protocol::flex_lc_config_reply& lc_config_update) {
  update_liveness();
  if (lc_config_update.lc_ue_config_size() == 0)
    return;
  lc_config_mutex_.lock();
  lc_config_.CopyFrom(lc_config_update);
  lc_config_mutex_.unlock();
}

void flexran::rib::enb_rib_info::update_subframe(const protocol::flex_sf_trigger& sf_trigger) {
  rnti_t rnti;
  uint16_t sfn_sf = sf_trigger.sfn_sf();
  current_frame_ = get_frame(sfn_sf);
  current_subframe_ = get_subframe(sfn_sf);

  // Update dl_sf_info
  for (int i = 0; i < sf_trigger.dl_info_size(); i++) {
    rnti = sf_trigger.dl_info(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
    } else {
      LOG4CXX_DEBUG(flog::rib, "update DL subframe info for RNTI " << rnti);
      it->second->update_dl_sf_info(sf_trigger.dl_info(i));
    }
  }

  // Update ul_sf_info
  for (int i = 0; i < sf_trigger.ul_info_size(); i++){ 
    rnti = sf_trigger.ul_info(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
    } else {
      LOG4CXX_DEBUG(flog::rib, "update UL subframe info for RNTI " << rnti);
      it->second->update_ul_sf_info(sf_trigger.ul_info(i));
    }
  }
  update_liveness();
}

void flexran::rib::enb_rib_info::update_mac_stats(const protocol::flex_stats_reply& mac_stats) {
  rnti_t rnti;
  // First make the UE updates
  for (int i = 0; i < mac_stats.ue_report_size(); i++) {
    rnti = mac_stats.ue_report(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
      //      ue_mac_info_.insert(std::pair<int,
      //std::shared_ptr<ue_mac_rib_info>>(rnti,
      //							    std::shared_ptr<ue_mac_rib_info>(new ue_mac_rib_info(rnti))));
    } else {
      it->second->update_mac_stats_report(mac_stats.ue_report(i));
      LOG4CXX_DEBUG(flog::rib, "Update MAC stats for RNTI " << rnti);
    }
  }
  // Then work on the Cell updates
  for (int i = 0; i < mac_stats.cell_report_size(); i++) {
    cell_mac_info_[i].update_cell_stats_report(mac_stats.cell_report(i));
  }
}

std::shared_ptr<flexran::rib::ue_mac_rib_info> flexran::rib::enb_rib_info::get_ue_mac_info(rnti_t rnti) const
{
  auto it = ue_mac_info_.find(rnti);
  if (it != ue_mac_info_.end()) {
    return it->second;
  }
  return std::shared_ptr<ue_mac_rib_info>(nullptr);
}

bool flexran::rib::enb_rib_info::need_to_query() {
  st_clock::duration dur = st_clock::now() - last_checked;
  return (dur > time_to_query);
}

void flexran::rib::enb_rib_info::update_liveness() {
  last_checked = st_clock::now();
}

void flexran::rib::enb_rib_info::dump_mac_stats() const {
  LOG4CXX_INFO(flog::rib, "UE MAC stats for BS " << bs_id_);
  for (auto ue_stats : ue_mac_info_) {
    ue_stats.second->dump_stats();
  }
}

std::string flexran::rib::enb_rib_info::dump_mac_stats_to_string() const {
  std::string str;

  str += "UE MAC stats for BS ";
  str += bs_id_;
  str += "\n";
  for (auto ue_stats : ue_mac_info_) {
    str += ue_stats.second->dump_stats_to_string();
    str += "\n";
  }

  return str;
}

std::string flexran::rib::enb_rib_info::dump_mac_stats_to_json_string() const
{
  std::vector<std::string> ue_mac_stats;
  ue_mac_stats.reserve(ue_mac_info_.size());
  std::transform(ue_mac_info_.begin(), ue_mac_info_.end(), std::back_inserter(ue_mac_stats),
      [] (const std::pair<rnti_t, std::shared_ptr<ue_mac_rib_info>>& ue_stats)
      { return ue_stats.second->dump_stats_to_json_string(); }
  );

  return format_mac_stats_to_json(bs_id_, ue_mac_stats);
}

std::string flexran::rib::enb_rib_info::format_mac_stats_to_json(
    uint64_t bs_id,
    const std::vector<std::string>& ue_mac_stats_json)
{
  std::string str;
  str += "\"bs_id\":";
  str += std::to_string(bs_id);
  str += ",\"ue_mac_stats\":[";
  for (auto it = ue_mac_stats_json.begin(); it != ue_mac_stats_json.end(); it++) {
    if (it != ue_mac_stats_json.begin()) str += ",";
    str += *it;
  }
  str += "]";
  return str;
}

void flexran::rib::enb_rib_info::dump_configs() const {
  LOG4CXX_INFO(flog::rib, "dump_configs() for BS " << bs_id_);
  for (auto a : agents_)
    LOG4CXX_INFO(flog::rib, a->to_string());
  eNB_config_mutex_.lock();
  LOG4CXX_INFO(flog::rib, eNB_config_.DebugString());
  eNB_config_mutex_.unlock();
  ue_config_mutex_.lock();
  LOG4CXX_INFO(flog::rib, ue_config_.DebugString());
  ue_config_mutex_.unlock();
  lc_config_mutex_.lock();
  LOG4CXX_INFO(flog::rib, lc_config_.DebugString());
  lc_config_mutex_.unlock();
}

std::string flexran::rib::enb_rib_info::dump_configs_to_string() const {
  std::string str;
  str += "configs for BS " + std::to_string(bs_id_) + "\n";
  for (auto a : agents_)
    str += a->to_string() + "\n";
  eNB_config_mutex_.lock();
  str += eNB_config_.DebugString();
  eNB_config_mutex_.unlock();
  str += "\n";
  ue_config_mutex_.lock();
  str += ue_config_.DebugString();
  ue_config_mutex_.unlock();
  str += "\n";
  lc_config_mutex_.lock();
  str += lc_config_.DebugString();
  lc_config_mutex_.unlock();
  str += "\n";

  return str;
}

std::string flexran::rib::enb_rib_info::dump_configs_to_json_string() const
{
  std::string agent_info, enb_config, ue_config, lc_config;

  agent_info = "[";
  for (auto it = agents_.begin(); it != agents_.end(); ++it) {
    if (it != agents_.begin()) agent_info += ",";
    agent_info += (*it)->to_json();
  }
  agent_info += "]";

  eNB_config_mutex_.lock();
  google::protobuf::util::MessageToJsonString(eNB_config_, &enb_config, google::protobuf::util::JsonPrintOptions());
  eNB_config_mutex_.unlock();

  ue_config_mutex_.lock();
  google::protobuf::util::MessageToJsonString(ue_config_, &ue_config, google::protobuf::util::JsonPrintOptions());
  ue_config_mutex_.unlock();

  lc_config_mutex_.lock();
  google::protobuf::util::MessageToJsonString(lc_config_, &lc_config, google::protobuf::util::JsonPrintOptions());
  lc_config_mutex_.unlock();

  return format_configs_to_json(bs_id_, agent_info, enb_config, ue_config, lc_config);
}

std::string flexran::rib::enb_rib_info::format_configs_to_json(
    uint64_t bs_id,
    const std::string& agent_info_json,
    const std::string& eNB_config_json,
    const std::string& ue_config_json,
    const std::string& lc_config_json)
{
  std::string str;
  str += "\"bs_id\":";
  str += std::to_string(bs_id);
  str += ",\"agent_info\":";
  str += agent_info_json;
  str += ",\"eNB\":";
  str += eNB_config_json;
  str += ",\"UE\":";
  str += ue_config_json;
  str += ",\"LC\":";
  str += lc_config_json;
  return str;
}

bool flexran::rib::enb_rib_info::dump_ue_spec_stats_by_rnti_to_json_string(rnti_t rnti, std::string& out) const
{
  auto it = ue_mac_info_.find(rnti);
  if (it == ue_mac_info_.end()) return false;

  out = it->second->dump_stats_to_json_string();
  return true;
}

bool flexran::rib::enb_rib_info::parse_rnti_imsi(const std::string& rnti_imsi_s,
    rnti_t& rnti) const
{
  if (rnti_imsi_s.length() >= RNTI_ID_LENGTH_LIMIT) { // assume it is an IMSI
    uint64_t imsi;
    try {
      imsi = std::stoll(rnti_imsi_s);
    } catch (const std::invalid_argument& e) {
      return false;
    }
    return get_rnti(imsi, rnti);
  }

  // assume it is an RNTI
  try {
    rnti = std::stoi(rnti_imsi_s);
  } catch (const std::invalid_argument& e) {
    return false;
  }
  return ue_mac_info_.find(rnti) != ue_mac_info_.end();
}

bool flexran::rib::enb_rib_info::get_rnti(uint64_t imsi, rnti_t& rnti) const
{
  std::lock_guard<std::mutex> lg(ue_config_mutex_);
  for (int i = 0; i < ue_config_.ue_config_size(); i++) {
    if (ue_config_.ue_config(i).has_imsi()
        && imsi == ue_config_.ue_config(i).imsi()) {
      rnti = ue_config_.ue_config(i).rnti();
      return true;
    }
  }
  return false;
}

bool flexran::rib::enb_rib_info::has_dl_slice(uint32_t slice_id, uint16_t cell_id) const
{
  std::lock_guard<std::mutex> lg(eNB_config_mutex_);
  const protocol::flex_slice_config& s = eNB_config_.cell_config(cell_id).slice_config();
  for (int i = 0; i < s.dl_size(); i++) {
    if (s.dl(i).id() == slice_id) {
      return true;
    }
  }
  return false;
}

uint32_t flexran::rib::enb_rib_info::num_dl_slices(uint16_t cell_id) const
{
  std::lock_guard<std::mutex> lg(eNB_config_mutex_);
  return eNB_config_.cell_config(cell_id).slice_config().dl_size();
}

bool flexran::rib::enb_rib_info::has_ul_slice(uint32_t slice_id, uint16_t cell_id) const
{
  std::lock_guard<std::mutex> lg(eNB_config_mutex_);
  const protocol::flex_slice_config& s = eNB_config_.cell_config(cell_id).slice_config();
  for (int i = 0; i < s.ul_size(); i++) {
    if (s.ul(i).id() == slice_id) {
      return true;
    }
  }
  return false;
}

uint32_t flexran::rib::enb_rib_info::num_ul_slices(uint16_t cell_id) const
{
  std::lock_guard<std::mutex> lg(eNB_config_mutex_);
  return eNB_config_.cell_config(cell_id).slice_config().ul_size();
}

/*
 * Clear all repeated fields in protobuf message dst which are present in src
 */
void flexran::rib::enb_rib_info::clear_repeated_if_present(
    google::protobuf::Message *dst, const google::protobuf::Message& src)
{
  const google::protobuf::Descriptor *desc_src = src.GetDescriptor();
  const google::protobuf::Reflection *refl_src = src.GetReflection();
  const google::protobuf::Descriptor *desc_dst = dst->GetDescriptor();
  const google::protobuf::Reflection *refl_dst = dst->GetReflection();
  for (int i = 0; i < desc_src->field_count(); ++i) {
    const google::protobuf::FieldDescriptor *field_src = desc_src->field(i);
    if (!field_src) continue;
    if (!field_src->is_repeated()) continue;
    if (refl_src->FieldSize(src, field_src) == 0) continue;

    const google::protobuf::FieldDescriptor *field_dst = desc_dst->field(i);
    if (!field_dst) continue;
    if (!field_dst->is_repeated()) continue;
    refl_dst->ClearField(dst, field_dst);
  }
}

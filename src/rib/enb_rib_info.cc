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


#include <google/protobuf/util/json_util.h>


#include "enb_rib_info.h"
#include "flexran_log.h"



flexran::rib::enb_rib_info::enb_rib_info(int agent_id)
  : agent_id_(agent_id) {
  last_checked = st_clock::now();
}

void flexran::rib::enb_rib_info::update_eNB_config(const protocol::flex_enb_config_reply& enb_config_update) {
  eNB_config_mutex_.lock();
  eNB_config_.CopyFrom(enb_config_update);
  eNB_config_mutex_.unlock();
  update_liveness();
}

void flexran::rib::enb_rib_info::update_UE_config(const protocol::flex_ue_config_reply& ue_config_update) {
  ue_config_mutex_.lock();
  ue_config_.CopyFrom(ue_config_update);
  ue_config_mutex_.unlock();
  rnti_t rnti;
  
  // Check if UE exists and if not create a ue_mac_rib_info entry
  for (int i = 0; i < ue_config_update.ue_config_size(); i++) {
    rnti = ue_config_update.ue_config(i).rnti();
    LOG4CXX_DEBUG(flog::rib, "update UE config for RNTI " << rnti);
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      ue_mac_info_.insert(std::pair<int,
			  std::shared_ptr<ue_mac_rib_info>>(rnti,
							    std::shared_ptr<ue_mac_rib_info>(new ue_mac_rib_info(rnti))));
    }
  }
  update_liveness();
}

void flexran::rib::enb_rib_info::update_UE_config(const protocol::flex_ue_state_change& ue_state_change) {
  rnti_t rnti;  
  // releases itself when leaving scope
  std::lock_guard<std::mutex> lg(ue_config_mutex_);
  if (ue_state_change.type() == protocol::FLUESC_ACTIVATED) {
    protocol::flex_ue_config *c = ue_config_.add_ue_config();
    c->CopyFrom(ue_state_change.config());
    rnti = ue_state_change.config().rnti();
    ue_mac_info_.insert(std::pair<int,
			std::shared_ptr<ue_mac_rib_info>>(rnti,
							  std::shared_ptr<ue_mac_rib_info>(new ue_mac_rib_info(rnti))));
    return;
  }
  for (int i = 0; i < ue_config_.ue_config_size(); i++) {
    rnti = ue_config_.ue_config(i).rnti();
    if (rnti == ue_state_change.config().rnti()) {
      // Check if this was updated or removed
      if (ue_state_change.type() == protocol::FLUESC_DEACTIVATED) {
	ue_config_.mutable_ue_config()->DeleteSubrange(i, 1);
	// Erase mac info
	ue_mac_info_.erase(rnti);
	// Erase lc info as well
        lc_config_mutex_.lock();
	for (int j = 0; j < lc_config_.lc_ue_config_size(); j++) {
	  if (rnti == lc_config_.lc_ue_config(j).rnti()) {
	    lc_config_.mutable_lc_ue_config()->DeleteSubrange(j, 1);
	  }
	}
        lc_config_mutex_.unlock();
	return;
      } else if (ue_state_change.type() == protocol::FLUESC_UPDATED) {
	ue_config_.mutable_ue_config(i)->CopyFrom(ue_state_change.config());
      }
    }
  }
}

void flexran::rib::enb_rib_info::update_LC_config(const protocol::flex_lc_config_reply& lc_config_update) {
  lc_config_mutex_.lock();
  lc_config_.CopyFrom(lc_config_update);
  lc_config_mutex_.unlock();
  update_liveness();
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
  LOG4CXX_INFO(flog::rib, "UE MAC stats for agent " << agent_id_);
  for (auto ue_stats : ue_mac_info_) {
    ue_stats.second->dump_stats();
  }
}

std::string flexran::rib::enb_rib_info::dump_mac_stats_to_string() const {
  std::string str;

  str += "UE MAC stats for agent ";
  str += agent_id_;
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

  return format_mac_stats_to_json(agent_id_, eNB_config_.enb_id(), ue_mac_stats);
}

std::string flexran::rib::enb_rib_info::format_mac_stats_to_json(int agent_id,
    uint64_t enb_id,
    const std::vector<std::string>& ue_mac_stats_json)
{
  std::string str;
  str += "\"agent_id\":";
  str += std::to_string(agent_id);
  str += ",\"eNBId\":";
  str += std::to_string(enb_id);
  str += ",\"ue_mac_stats\":[";
  for (auto it = ue_mac_stats_json.begin(); it != ue_mac_stats_json.end(); it++) {
    if (it != ue_mac_stats_json.begin()) str += ",";
    str += *it;
  }
  str += "]";
  return str;
}

void flexran::rib::enb_rib_info::dump_configs() const {
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
  std::string enb_config, ue_config, lc_config;
  uint64_t enb_id;

  eNB_config_mutex_.lock();
  google::protobuf::util::MessageToJsonString(eNB_config_, &enb_config, google::protobuf::util::JsonPrintOptions());
  enb_id = eNB_config_.enb_id();
  eNB_config_mutex_.unlock();

  ue_config_mutex_.lock();
  google::protobuf::util::MessageToJsonString(ue_config_, &ue_config, google::protobuf::util::JsonPrintOptions());
  ue_config_mutex_.unlock();

  lc_config_mutex_.lock();
  google::protobuf::util::MessageToJsonString(lc_config_, &lc_config, google::protobuf::util::JsonPrintOptions());
  lc_config_mutex_.unlock();

  return format_configs_to_json(agent_id_, enb_id, enb_config, ue_config, lc_config);
}

std::string flexran::rib::enb_rib_info::format_configs_to_json(
    int agent_id, uint64_t enb_id,
    const std::string& eNB_config_json,
    const std::string& ue_config_json,
    const std::string& lc_config_json)
{
  std::string str;
  str += "\"agent_id\":";
  str += std::to_string(agent_id);
  str += ",\"eNBId\":";
  str += std::to_string(enb_id);
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
    } catch (std::invalid_argument e) {
      return false;
    }
    return get_rnti(imsi, rnti);
  }

  // assume it is an RNTI
  try {
    rnti = std::stoi(rnti_imsi_s);
  } catch (std::invalid_argument e) {
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

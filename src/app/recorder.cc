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

/*! \file    recorder.cc
 *  \brief   app for real-time statistics recording
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <fstream>
#include <thread>
#include <iomanip>

#include <google/protobuf/util/json_util.h>

#include "recorder.h"
#include "enb_rib_info.h"
#include "flexran_log.h"

bool flexran::app::log::bs_dump::operator==(const bs_dump& other) const
{
  if (ue_mac_harq_infos.size() != other.ue_mac_harq_infos.size()) return false;
  for (auto it = ue_mac_harq_infos.begin(), oit = other.ue_mac_harq_infos.begin();
        it != ue_mac_harq_infos.end(); it++, oit++) {
    // if other iterator over ue_mac_harq_infos reaches end before "local" one,
    // there is a problem
    if (oit == other.ue_mac_harq_infos.end()) return false;
    // test harq array equality
    if (it->second != oit->second) return false;
    // compare that serialized flex_ue_stats_report messages are the same
    std::string ss1, ss2;
    if (!it->first.SerializeToString(&ss1)) return false;
    if (!it->first.SerializeToString(&ss2)) return false;
    if (ss1 != ss2) return false;
  }
  // compare that serialized flex_*_config_reply messages are the same
  std::string sec1, sec2, suc1, suc2, slc1, slc2;
  if (!enb_config.SerializeToString(&sec1)) return false;
  if (!other.enb_config.SerializeToString(&sec2)) return false;
  if (!ue_config.SerializeToString(&suc1)) return false;
  if (!other.ue_config.SerializeToString(&suc2)) return false;
  if (!lc_config.SerializeToString(&slc1)) return false;
  if (!other.lc_config.SerializeToString(&slc2)) return false;
  return sec1 == sec2 && suc1 == suc2 && slc1 == slc2;
}

bool flexran::app::log::recorder::start_meas(uint64_t duration,
    const std::string& type, std::string& id)
{
  /* in case we are currently in a measurement, do not handle */
  if (current_job_)
    LOG4CXX_DEBUG(flog::app, "recorder: job already running, starting next immediately after");

  job_type jt;
  if      (type == "all")   jt = job_type::all;
  else if (type == "enb")   jt = job_type::enb;
  else if (type == "stats") jt = job_type::stats;
  else if (type == "bin")   jt = job_type::bin;
  else {
    LOG4CXX_ERROR(flog::app, "recorder: illegal job type " << type);
    return false;
  }

  if (duration < 1) {
    LOG4CXX_ERROR(flog::app, "recorder: duration must be larger than 0");
    return false;
  }

  /* write dummy data to hopefully fill caches (twice, intentionally) */
  for (uint64_t bs_id: rib_.get_available_base_stations()) {
    record_chunk(bs_id);
    record_chunk(bs_id);
  }

  /* ID corresponds to record start date, but first check if we can have the
   * corresponding file */
  const uint64_t start = current_job_ ? current_job_->ms_end : event_sub_.last_tick() + 2;
  id = std::to_string(start);
  std::string filename = "/tmp/record." + id + ".json";
  if (jt == job_type::bin)
    filename += ".bin";

  /* test whether we can write (later) */
  std::ofstream file;
  file.open(filename);
  if (!file.is_open()) {
    LOG4CXX_ERROR(flog::app, "recorder: cannot open file " << filename);
    return false;
  }
  file.close();


  dump_.reset(new std::vector<std::map<uint64_t, bs_dump>>);
  dump_->reserve(duration);
  current_job_.reset(new job_info{start, start + duration, filename, jt});

  std::chrono::duration<float, std::ratio<60l>> min = std::chrono::milliseconds(duration);
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  LOG4CXX_INFO(flog::app, "recorder: created job " << start << " (for "
      << min.count() << "min@" << std::put_time(std::localtime(&now), "%T")
      << ", file " << filename << ", type " << type << ")");

  event_sub_.subscribe_task_tick_extended(
      boost::bind(&flexran::app::log::recorder::tick, this, _1, _2), 1, start);

  return true;
}

bool flexran::app::log::recorder::get_job_info(const std::string& id, job_info& info)
{
  auto it = std::find_if(finished_jobs_.begin(), finished_jobs_.end(),
      [&id] (const job_info& ji) { return id == std::to_string(ji.ms_start); }
  );
  if (it == finished_jobs_.end()) return false;

  info = *it;
  return true;
}

void flexran::app::log::recorder::tick(const bs2::connection& conn, uint64_t ms)
{
  if (!current_job_) {
    LOG4CXX_ERROR(flog::app, "no current job available");
    conn.disconnect();
    return;
  }

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::map<uint64_t, flexran::app::log::bs_dump> m;
  for (uint64_t bs_id: rib_.get_available_base_stations()) {
    m.insert(std::make_pair(bs_id, record_chunk(bs_id)));
  }
  dump_->push_back(m);
  std::chrono::duration<float, std::micro> dur = std::chrono::steady_clock::now() - start;
  LOG4CXX_TRACE(flog::app, "write_json_chunk() at " << ms
      << "ms, duration " << dur.count() << "us");

  if (ms == current_job_->ms_end - 1) {
    std::thread writer(&flexran::app::log::recorder::writer_method, this,
        std::move(current_job_), std::move(dump_));
    writer.detach();
    conn.disconnect();
    current_job_.reset();
  }
}

flexran::app::log::bs_dump flexran::app::log::recorder::record_chunk(uint64_t bs_id)
{
  std::vector<mac_harq_info_t> ue_mac_harq_infos;
  const protocol::flex_ue_config_reply& ue_configs = rib_.get_bs(bs_id)->get_ue_configs();
  for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
    flexran::rib::rnti_t rnti = ue_configs.ue_config(UE_id).rnti();
    std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = rib_.get_bs(bs_id)->get_ue_mac_info(rnti);
    std::array<bool, 8> harq_infos;
    auto& harq_array = ue_mac_info->get_all_harq_stats();
    for (int i = 0; i < 8; i++) {
      harq_infos[i] = harq_array[0][i][0] == protocol::FLHS_ACK;
    }
    ue_mac_harq_infos.push_back(std::make_pair(ue_mac_info->get_mac_stats_report(), harq_infos));
  }

  return flexran::app::log::bs_dump {
    rib_.get_bs(bs_id)->get_enb_config(),
    rib_.get_bs(bs_id)->get_ue_configs(),
    rib_.get_bs(bs_id)->get_lc_configs(),
    ue_mac_harq_infos
  };
}

void flexran::app::log::recorder::writer_method(std::unique_ptr<job_info> info,
    std::unique_ptr<std::vector<std::map<uint64_t, bs_dump>>> dump)
{
  uint64_t n;
  if (info->type == job_type::bin)
    n = write_binary(*info, *dump);
  else
    n = write_json(*info, *dump);

  std::chrono::duration<float, std::ratio<60l>> min = std::chrono::milliseconds(n);
  LOG4CXX_INFO(flog::app, "recorder: " << n
      << " objects persisted, corresponding to " << min.count() << " min");
  finished_jobs_.push_back(*info);
  info.release();
  dump.release();
}

uint64_t flexran::app::log::recorder::write_json(job_info info,
    const std::vector<std::map<uint64_t, bs_dump>>& dump)
{
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::ofstream file;
  file.open(info.filename);
  if (!file.is_open()) {
    LOG4CXX_ERROR(flog::app, "recorder: cannot open file " << info.filename);
    return 0;
  }
  file << "[";
  for (auto it = dump.begin(); it != dump.end(); it++) {
    if (it != dump.begin()) file << ",";
    write_json_chunk(file, info.type, *it);
  }
  file << "]";
  file.close();
  std::chrono::duration<float, std::milli> dur = std::chrono::steady_clock::now() - start;
  LOG4CXX_INFO(flog::app, "recorder: wrote JSON to file " << info.filename
      << " (in " << dur.count() << " ms)");
  return std::distance(dump.begin(), dump.end());
}

void flexran::app::log::recorder::write_json_chunk(std::ostream& s,
    job_type type,
    const std::map<uint64_t, bs_dump>& dump_chunk)
{
  /* TODO use flexran::rib::Rib::format_statistics_to_json() when time stamps
   * for single chunks are collected */
  s << "{";

  if (type != job_type::stats) {
    std::vector<std::string> enb_configurations;
    enb_configurations.reserve(dump_chunk.size());
    std::transform(dump_chunk.begin(), dump_chunk.end(), std::back_inserter(enb_configurations),
        [] (const std::pair<uint64_t, bs_dump>& p)
        {
          const bs_dump& bd = p.second;
          std::string enb_config, ue_config, lc_config;
          google::protobuf::util::MessageToJsonString(bd.enb_config, &enb_config,
              google::protobuf::util::JsonPrintOptions());
          google::protobuf::util::MessageToJsonString(bd.ue_config, &ue_config,
              google::protobuf::util::JsonPrintOptions());
          google::protobuf::util::MessageToJsonString(bd.lc_config, &lc_config,
              google::protobuf::util::JsonPrintOptions());
          return flexran::rib::enb_rib_info::format_configs_to_json(
              p.first, "\"null\"", enb_config, ue_config, lc_config);
        }
    );
    s << flexran::rib::Rib::format_enb_configurations_to_json(enb_configurations);
  }

  if (type == job_type::all) {
    s << ",";
  }

  if (type != job_type::enb) {
    std::vector<std::string> ue_stats;
    ue_stats.reserve(ue_stats.size());
    std::transform(dump_chunk.begin(), dump_chunk.end(), std::back_inserter(ue_stats),
        [] (const std::pair<uint64_t, bs_dump>& p)
        {
          const uint64_t bs_id = p.first;
          std::vector<std::string> ue_stats = get_ue_stats(p.second.ue_mac_harq_infos);
          return flexran::rib::enb_rib_info::format_mac_stats_to_json(bs_id, ue_stats);
        }
    );
    s << flexran::rib::Rib::format_mac_stats_to_json(ue_stats);
  }
  s << "}";
}

std::vector<std::string> flexran::app::log::recorder::get_ue_stats(
    const std::vector<mac_harq_info_t>& ue_mac_harq_infos)
{
  std::vector<std::string> ue_mac_stats;
  ue_mac_stats.reserve(ue_mac_harq_infos.size());
  std::transform(ue_mac_harq_infos.begin(), ue_mac_harq_infos.end(), std::back_inserter(ue_mac_stats),
      [] (const mac_harq_info_t& ue_mac_harq_info)
      {
        const protocol::flex_ue_stats_report& ue_config = ue_mac_harq_info.first;
        std::string mac_stats;
        google::protobuf::util::MessageToJsonString(ue_config, &mac_stats, google::protobuf::util::JsonPrintOptions());

        std::array<std::string, 8> harq;
        for (int i = 0; i < 8; i++) {
          harq[i] = ue_mac_harq_info.second[i] ? "\"ACK\"" : "\"NACK\"";
        }

        return flexran::rib::ue_mac_rib_info::format_stats_to_json(ue_config.rnti(),
                mac_stats, harq);
      }
  );
  return ue_mac_stats;
}

uint64_t flexran::app::log::recorder::write_binary(job_info info,
    const std::vector<std::map<uint64_t, bs_dump>>& dump)
{
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::ofstream file;
  file.open(info.filename, std::ios::binary);
  if (!file.is_open()) {
    LOG4CXX_ERROR(flog::app, "recorder: cannot open file " << info.filename);
    return 0;
  }
  uint64_t n = std::distance(dump.begin(), dump.end());
  file.write(reinterpret_cast<const char *>(&n), sizeof(uint64_t));
  for (auto it = dump.begin(); it != dump.end(); it++) {
    write_binary_chunk(file, *it);
  }
  file.close();
  std::chrono::duration<float, std::milli> dur = std::chrono::steady_clock::now() - start;
  LOG4CXX_INFO(flog::app, "recorder: serialized into file " << info.filename
      << " (in " << dur.count() << " ms)");

  return n;
}

void flexran::app::log::recorder::write_binary_chunk(std::ostream& s,
    const std::map<uint64_t, bs_dump>& dump_chunk)
{
  uint16_t n = std::distance(dump_chunk.begin(), dump_chunk.end());
  s.write(reinterpret_cast<const char *>(&n), sizeof(uint16_t));
  for (auto it = dump_chunk.begin(); it != dump_chunk.end(); it++) {
    s.write(reinterpret_cast<const char *>(&it->first), sizeof(uint64_t));
    const bs_dump& bd = it->second;
    if (!write_flexran_message(s, bd.enb_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_enb_config_reply");
      return;
    }
    if (!write_flexran_message(s, bd.ue_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_ue_config_reply");
      return;
    }
    if (!write_flexran_message(s, bd.lc_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_lc_config_reply");
      return;
    }
    write_binary_ue_configs(s, bd.ue_mac_harq_infos);
  }
}

void flexran::app::log::recorder::write_binary_ue_configs(std::ostream& s,
    const std::vector<mac_harq_info_t>& ue_mac_harq_infos)
{
  uint16_t n = std::distance(ue_mac_harq_infos.begin(), ue_mac_harq_infos.end());
  s.write(reinterpret_cast<const char *>(&n), sizeof(uint16_t));
  for (auto it = ue_mac_harq_infos.begin(); it != ue_mac_harq_infos.end(); it++) {
    const protocol::flex_ue_stats_report& ue_config = it->first;
    if (!write_flexran_message(s, ue_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_ue_stats_report");
      return;
    }
    uint8_t harq = 0;
    const std::array<bool, 8>& harq_arr = it->second;
    for (int i = 0; i < 8; i++)
      harq |= (harq_arr[i] << i);
    s.write(reinterpret_cast<const char *>(&harq), sizeof(uint8_t));
  }
}

std::vector<std::map<uint64_t, flexran::app::log::bs_dump>>
flexran::app::log::recorder::read_binary(std::string filename)
{
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::vector<std::map<uint64_t, flexran::app::log::bs_dump>> dump;
  std::ifstream file;
  file.open(filename, std::ios::binary);
  if (!file.is_open()) {
    LOG4CXX_ERROR(flog::app, "recorder: cannot open file " << filename);
    return dump;
  }

  uint64_t n;
  file.read(reinterpret_cast<char *>(&n), sizeof(uint64_t));

  for (uint64_t i = 0; i < n; i++) {
    dump.push_back(read_binary_chunk(file));
  }
  file.close();
  std::chrono::duration<float, std::milli> dur = std::chrono::steady_clock::now() - start;
  LOG4CXX_INFO(flog::app, "recorder: deserialized from file " << filename
      << " (in " << dur.count() << " ms)");

  return dump;
}

std::map<uint64_t, flexran::app::log::bs_dump>
flexran::app::log::recorder::read_binary_chunk(std::istream &s)
{
  std::map<uint64_t, flexran::app::log::bs_dump> dump_chunk;
  uint16_t n;
  s.read(reinterpret_cast<char *>(&n), sizeof(uint16_t));
  for (uint16_t i = 0; i < n; i++) {
    uint64_t bs_id;
    s.read(reinterpret_cast<char *>(&bs_id), sizeof(uint64_t));
    protocol::flex_enb_config_reply enb_config;
    protocol::flex_ue_config_reply ue_config;
    protocol::flex_lc_config_reply lc_config;
    if (!read_flexran_message(s, enb_config)) {
      LOG4CXX_ERROR(flog::app,
          "error while reading binary flex_enb_config_reply in pass "
          << i << ", trying to continue");
    }
    if (!read_flexran_message(s, ue_config)) {
      LOG4CXX_ERROR(flog::app,
          "error while reading binary flex_ue_config_reply in pass "
          << i << ", trying to continue");
    }
    if (!read_flexran_message(s, lc_config)) {
      LOG4CXX_ERROR(flog::app,
          "error while reading binary flex_lc_config_reply in pass "
          << i << ", trying to continue");
    }

    dump_chunk.emplace(bs_id,
        flexran::app::log::bs_dump {
          enb_config,
          ue_config,
          lc_config,
          read_binary_ue_configs(s)
        }
    );
  }

  return dump_chunk;
}

std::vector<mac_harq_info_t>
flexran::app::log::recorder::read_binary_ue_configs(std::istream &s)
{
  std::vector<mac_harq_info_t> ue_mac_harq_infos;
  uint16_t n;
  s.read(reinterpret_cast<char *>(&n), sizeof(uint16_t));
  for (uint16_t i = 0; i < n; i++) {
    protocol::flex_ue_stats_report ue_config;
    if (!read_flexran_message(s, ue_config)) {
      LOG4CXX_ERROR(flog::app, "error while reading binary flex_ue_stats_report, trying to continue");
    }
    uint8_t harq_byte;
    s.read(reinterpret_cast<char *>(&harq_byte), sizeof(uint8_t));
    std::array<bool, 8> harq;
    for (int j = 0; j < 8; j++)
      harq[j] = (harq_byte & (1 << j)) >> j;
    ue_mac_harq_infos.push_back(std::make_pair(ue_config, harq));
  }
  return ue_mac_harq_infos;
}

template <typename T>
bool flexran::app::log::recorder::write_flexran_message(std::ostream& s, const T& flex_message)
{
  size_t n = flex_message.ByteSizeLong();
  s.write(reinterpret_cast<const char *>(&n), sizeof(size_t));
  return flex_message.SerializeToOstream(&s);
}

template <typename T>
bool flexran::app::log::recorder::read_flexran_message(std::istream& s, T& flex_message)
{
  size_t n;
  s.read(reinterpret_cast<char *>(&n), sizeof(size_t));

  std::string bytes(n, 0); // delimit with zero
  // read number of bytes corresponding to message into buffer
  // https://developers.google.com/protocol-buffers/docs/techniques#streaming
  s.read(&bytes[0], n);

  return flex_message.ParseFromString(bytes);
}

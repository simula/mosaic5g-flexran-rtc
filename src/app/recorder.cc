/* The MIT License (MIT)

   Copyright (c) 2018 Robert Schmidt

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <fstream>
#include <thread>
#include <iomanip>

#include <google/protobuf/util/json_util.h>

#include "recorder.h"
#include "enb_rib_info.h"
#include "flexran_log.h"

bool flexran::app::log::recorder::start_meas(uint64_t duration,
    const std::string& type, std::string& id)
{
  /* in case we are currently in a measurement, do not handle */
  if (current_job_
      && ms_counter_ >= current_job_->ms_start - 2
      && ms_counter_ <  current_job_->ms_end) {
    LOG4CXX_DEBUG(flog::app, "recorder: job already running");
    return false;
  }

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
  for (int agent_id: rib_.get_available_agents()) {
    record_chunk(agent_id);
    record_chunk(agent_id);
  }

  /* ID corresponds to record start date, but first check if we can have the
   * corresponding file */
  uint64_t start = ms_counter_ + 2;
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


  dump_.reset(new std::vector<std::map<int, agent_dump>>);
  dump_->reserve(duration);
  current_job_.reset(new job_info{start, start + duration, filename, jt});

  std::chrono::duration<float, std::ratio<60l>> min = std::chrono::milliseconds(duration);
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  LOG4CXX_INFO(flog::app, "recorder: created job " << start << " (for "
      << min.count() << "min@" << std::put_time(std::localtime(&now), "%T")
      << ", file " << filename << ", type " << type << ")");

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

void flexran::app::log::recorder::periodic_task()
{
  ms_counter_++;

  if (!current_job_
      || ms_counter_ <  current_job_->ms_start
      || ms_counter_ >= current_job_->ms_end)
    return;

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::map<int, flexran::app::log::agent_dump> m;
  for (int agent_id: rib_.get_available_agents()) {
    m.insert(std::make_pair(agent_id, record_chunk(agent_id)));
  }
  dump_->push_back(m);
  std::chrono::duration<float, std::micro> dur = std::chrono::steady_clock::now() - start;
  LOG4CXX_TRACE(flog::app, "write_json_chunk() at " << ms_counter_ << ", duration " << dur.count() << "us");

  if (ms_counter_ == current_job_->ms_end - 1) {
    std::thread writer(&flexran::app::log::recorder::writer_method, this,
        std::move(current_job_), std::move(dump_));
    writer.detach();
    current_job_.reset();
  }
}

flexran::app::log::agent_dump flexran::app::log::recorder::record_chunk(int agent_id)
{
  std::vector<mac_harq_info_t> ue_mac_harq_infos;
  const protocol::flex_ue_config_reply& ue_configs = rib_.get_agent(agent_id)->get_ue_configs();
  for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
    flexran::rib::rnti_t rnti = ue_configs.ue_config(UE_id).rnti();
    std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = rib_.get_agent(agent_id)->get_ue_mac_info(rnti);
    std::array<bool, 8> harq_infos;
    auto& harq_array = ue_mac_info->get_all_harq_stats();
    for (int i = 0; i < 8; i++) {
      harq_infos[i] = harq_array[0][i][0] == protocol::FLHS_ACK;
    }
    ue_mac_harq_infos.push_back(std::make_pair(ue_mac_info->get_mac_stats_report(), harq_infos));
  }

  return flexran::app::log::agent_dump {
    rib_.get_agent(agent_id)->get_enb_config(),
    rib_.get_agent(agent_id)->get_ue_configs(),
    rib_.get_agent(agent_id)->get_lc_configs(),
    ue_mac_harq_infos
  };
}

void flexran::app::log::recorder::writer_method(std::unique_ptr<job_info> info,
    std::unique_ptr<std::vector<std::map<int, agent_dump>>> dump)
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
    const std::vector<std::map<int, agent_dump>>& dump)
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
    const std::map<int, agent_dump>& dump_chunk)
{
  s << "{";

  if (type != job_type::stats) {
    std::vector<std::string> enb_configurations;
    enb_configurations.reserve(dump_chunk.size());
    std::transform(dump_chunk.begin(), dump_chunk.end(), std::back_inserter(enb_configurations),
        [] (const std::pair<int, agent_dump>& p)
        {
          const agent_dump& ad = p.second;
          std::string enb_config, ue_config, lc_config;
          google::protobuf::util::MessageToJsonString(ad.enb_config, &enb_config,
              google::protobuf::util::JsonPrintOptions());
          google::protobuf::util::MessageToJsonString(ad.ue_config, &ue_config,
              google::protobuf::util::JsonPrintOptions());
          google::protobuf::util::MessageToJsonString(ad.lc_config, &lc_config,
              google::protobuf::util::JsonPrintOptions());
          return flexran::rib::enb_rib_info::format_configs_to_json(
              p.first, ad.enb_config.enb_id(), enb_config, ue_config, lc_config);
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
        [] (const std::pair<flexran::rib::rnti_t, agent_dump>& p)
        {
          int agent_id = p.first;
          uint64_t enb_id = p.second.enb_config.enb_id();
          std::vector<std::string> ue_stats = get_ue_stats(p.second.ue_mac_harq_infos);
          return flexran::rib::enb_rib_info::format_mac_stats_to_json(agent_id,
              enb_id, ue_stats);
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
    const std::vector<std::map<int, agent_dump>>& dump)
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

  /*
   * TEST mode:
   * can be used to check whether binary serialization is correct. This code
   * reads the binary data into new_dump and writes it to a json file. Then,
   * the original dump is written to a JSON file with an appended .orig. Then,
   * the record.\d\+.json{,.orig} files should be compared.
   */
  /*LOG4CXX_INFO(flog::app, "TEST: deserialize and write to JSON");
  std::vector<std::map<int, agent_dump>> new_dump;
  new_dump = read_binary(info.filename);

  info.type = job_type::all;
  write_json(info, new_dump);

  info.filename += ".orig";
  write_json(info, dump);*/

  return n;
}

void flexran::app::log::recorder::write_binary_chunk(std::ostream& s,
    const std::map<int, agent_dump>& dump_chunk)
{
  uint16_t n = std::distance(dump_chunk.begin(), dump_chunk.end());
  s.write(reinterpret_cast<const char *>(&n), sizeof(uint16_t));
  for (auto it = dump_chunk.begin(); it != dump_chunk.end(); it++) {
    s.write(reinterpret_cast<const char *>(&it->first), sizeof(int));
    const agent_dump& ad = it->second;
    if (!write_flexran_message(s, ad.enb_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_enb_config_reply");
      return;
    }
    if (!write_flexran_message(s, ad.ue_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_ue_config_reply");
      return;
    }
    if (!write_flexran_message(s, ad.lc_config)) {
      LOG4CXX_ERROR(flog::app, "error while writing binary flex_lc_config_reply");
      return;
    }
    write_binary_ue_configs(s, ad.ue_mac_harq_infos);
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

std::vector<std::map<int, flexran::app::log::agent_dump>>
flexran::app::log::recorder::read_binary(std::string filename)
{
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::vector<std::map<int, flexran::app::log::agent_dump>> dump;
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

std::map<int, flexran::app::log::agent_dump>
flexran::app::log::recorder::read_binary_chunk(std::istream &s)
{
  std::map<int, flexran::app::log::agent_dump> dump_chunk;
  uint16_t n;
  s.read(reinterpret_cast<char *>(&n), sizeof(uint16_t));
  for (uint16_t i = 0; i < n; i++) {
    int agent_id;
    s.read(reinterpret_cast<char *>(&agent_id), sizeof(int));
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

    dump_chunk.insert(std::make_pair(agent_id,
        flexran::app::log::agent_dump {
          enb_config,
          ue_config,
          lc_config,
          read_binary_ue_configs(s)
        }
    ));
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

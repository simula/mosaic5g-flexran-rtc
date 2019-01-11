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

/*! \file    elastic_search.h
 *  \brief   Elastic Search client for database feeding
 *  \authors Robert Schmidt, Berkay Köksal
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr, berkay.koksal@eurecom.fr
 */

#include "elastic_search.h"
#include "enb_rib_info.h"
#include "flexran_log.h"
#include "rt_controller_common.h"

#include <chrono>
#include <string>
#include <thread>
#include <curl/curl.h>
#include <regex>

flexran::app::log::elastic_search::elastic_search(const rib::Rib& rib,
    const core::requests_manager& rm, event::subscription& sub)
  : component(rib, rm, sub),
    active_since_(std::chrono::system_clock::now()),
    sent_packets_(0),
    freq_stats_ (100),
    freq_config_(5000),
    batch_stats_max_no_(100),
    batch_config_max_no_(5)
{
  elastic_search_ep_.push_back("localhost:9200"),
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl_multi_ = curl_multi_init();
}

flexran::app::log::elastic_search::~elastic_search()
{
  disable_logging();
  curl_multi_cleanup(curl_multi_);
  curl_global_cleanup();
}


void flexran::app::log::elastic_search::process_stats(uint64_t tick)
{
  _unused(tick);

  //Collect all UE stats from all BS and fill the bulk batch
  int ue_count = 0;
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<rib::enb_rib_info> bs_config = rib_.get_bs(bs_id);
    const auto& lueu = bs_config->get_ue_configs();
    ue_count += lueu.ue_config().size();
    for(auto& flex_ue_config : lueu.ue_config()) {
      const flexran::rib::rnti_t rnti = flex_ue_config.rnti();
      std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = bs_config->get_ue_mac_info(rnti);
      const std::string json = rib_.format_statistics_to_json(
          std::chrono::system_clock::now(),
          "",
          ue_mac_info->dump_stats_to_json_string());
      batch_stats_data_ += bulk_create_index("mac_stats", json);
    }
  }
  batch_stats_current_no_ += ue_count;

  if (batch_stats_current_no_ >= batch_stats_max_no_) {
    trigger_send(batch_stats_data_);
    initialise_batch_stats();
  }
}

void flexran::app::log::elastic_search::ue_disconnect(uint64_t bs_id, flexran::rib::rnti_t rnti)
{
  _unused(bs_id);
  _unused(rnti);

  int ue_count = 0;
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<rib::enb_rib_info> bs_config = rib_.get_bs(bs_id);
    ue_count += bs_config->get_ue_configs().ue_config().size();
  }
  /* if it is the last UE (new ue_count 0), send the batch off */
  if (ue_count == 0) {
    trigger_send(batch_stats_data_);
    initialise_batch_stats();
  }
}

void flexran::app::log::elastic_search::process_config(uint64_t tick)
{
  _unused(tick);

  const std::string s = rib_.format_statistics_to_json(
      std::chrono::system_clock::now(),
      rib_.dump_all_enb_configurations_to_json_string(),
      "");
  batch_config_data_ += bulk_create_index("enb_config", s);
  batch_config_current_no_++;

  if (batch_config_current_no_ >= batch_config_max_no_) {
    trigger_send(batch_config_data_);
    initialise_batch_config();
  }
}

void flexran::app::log::elastic_search::trigger_send(const std::string& s)
{
  /* place a new transfer handle in curl's transfer queue */
  for (const std::string& addr : elastic_search_ep_) {
    CURL *temp = curl_create_transfer(addr + "/_bulk", s);
    curl_multi_add_handle(curl_multi_, temp);
  }
  /* actual transfer happens in process_curl() */
}

std::string flexran::app::log::elastic_search::bulk_create_index(
    const std::string& index, const std::string& data)
{
  /*
  //One EXAMPLE curl bulk request
  curl -XPOST "http://localhost:9200/_bulk" -H 'Content-Type: application/json' -d'
  {"index":{"_index":"mac_stats-2018-08-28","_type":"json"}}
  {"date_time":"2018-08-28T10:56:15.776","mac_stats":[]}
  '
  */
  std::string s = "{";
  s += "\"index\":{\"_index\":\"";
  s += index;
  s += "\",\"_type\":\"json\"}}\n";
  s += data;
  s += "\n";
  return s;
}

CURL *flexran::app::log::elastic_search::curl_create_transfer(
    const std::string& addr, const std::string& data)
{
  CURL *curl1;
  curl1 = curl_easy_init();

  //Headers for bulk request
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "Content-Type: application/x-ndjson");
  curl_easy_setopt(curl1, CURLOPT_HTTPHEADER, headers);

  //Request options
  curl_easy_setopt(curl1, CURLOPT_URL, addr.c_str());
  curl_easy_setopt(curl1, CURLOPT_POSTFIELDSIZE, data.length());
  /* tell curl to copy string so we can override it afterwards */
  curl_easy_setopt(curl1, CURLOPT_COPYPOSTFIELDS, data.c_str());
  curl_easy_setopt(curl1, CURLOPT_POST, 1L);
  curl_easy_setopt(curl1, CURLOPT_VERBOSE, 0L);
  /* provide lambda that swallows all output. operator+ converts to function
   * pointer which is necessary since we call a C library */
  curl_easy_setopt(curl1, CURLOPT_WRITEFUNCTION,
      +[](char *, size_t, size_t nmemb, void *) { return nmemb; });

  return curl1;
}

void flexran::app::log::elastic_search::curl_release_handles()
{
  /* check finished transfers and remove/free the handles */
  CURLMsg *m;
  int n;
  do {
   m = curl_multi_info_read(curl_multi_, &n);
   if (m && m->msg == CURLMSG_DONE) {
     CURL *e = m->easy_handle;
     long code = 0;
     curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &code);
     if (code == 200) /* if ok */
       sent_packets_ += 1;
     curl_multi_remove_handle(curl_multi_, e);
     curl_easy_cleanup(e);
   }
  } while (m);
}

void flexran::app::log::elastic_search::process_curl(uint64_t tick)
{
  _unused(tick);

  int n;
  /* from documentation for curl_multi_perform(): "This function does not
   * require that there actually is any data available for reading or that data
   * can be written, it can be called just in case." */
  CURLMcode mc = curl_multi_perform(curl_multi_, &n);
  if (mc != CURLM_OK) {
    LOG4CXX_ERROR(flog::app, "CURL encountered a problem (" << mc << "), disabling logging");
    disable_logging();
  }

  curl_release_handles();
}

void flexran::app::log::elastic_search::wait_curl_end()
{
  /* finish all curl transfers in a blocking fashion, remove handles and return */
  int n;
  do {
    CURLMcode mc = curl_multi_perform(curl_multi_, &n);
    if (mc == CURLM_OK ) {
      // wait for activity, timeout or "nothing"
      int numfds;
      mc = curl_multi_wait(curl_multi_, NULL, 0, 1000, &numfds);
      if (mc != CURLM_OK) break;
    } else {
      break;
    }
  } while (n);

  curl_release_handles();
}

bool flexran::app::log::elastic_search::add_endpoint(const std::string& ep)
{
  /* check that ep format is either numerical IP or hostname, followed by port,
   * then a file name but we only test for the slash and anything afterwards
   * Double-escape \ so it is not interpreted */
  std::regex ip_plus_port("^(?:[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+|[a-zA-Z0-9-]+):\\d+$");
  if (!std::regex_match(ep, ip_plus_port)) return false;

  /* check that it is not already present */
  auto it = std::find(elastic_search_ep_.begin(), elastic_search_ep_.end(), ep);
  if (it != elastic_search_ep_.end()) return false;

  elastic_search_ep_.push_back(ep);
  return true;
}

bool flexran::app::log::elastic_search::remove_endpoint(const std::string& ep)
{
  auto it = std::find(elastic_search_ep_.begin(), elastic_search_ep_.end(), ep);
  if (it == elastic_search_ep_.end()) return false;
  elastic_search_ep_.erase(it);
  if (elastic_search_ep_.size() == 0 && is_active())
    disable_logging();
  return true;
}

bool flexran::app::log::elastic_search::set_freq_stats(int freq)
{
  /* needs to be between 1 and 1000. Use 0 to switch off */
  if (freq < 0 || freq > 1000) return false;
  freq_stats_ = freq;
  if (is_active()) {
    if (tick_stats_.connected())
      tick_stats_.disconnect();
    if (freq_stats_ > 0)
      tick_stats_ = event_sub_.subscribe_task_tick(
          boost::bind(&flexran::app::log::elastic_search::process_config, this, _1),
          freq_stats_, event_sub_.last_tick());
  }
  return true;
}

bool flexran::app::log::elastic_search::set_freq_config(int freq)
{
  /* needs to be larger than 0. Use 0 to switch off */
  if (freq < 0) return false;
  freq_config_ = freq;
  if (is_active()) {
    if (tick_config_.connected())
      tick_config_.disconnect();
    if (freq_config_ > 0)
      tick_config_ = event_sub_.subscribe_task_tick(
          boost::bind(&flexran::app::log::elastic_search::process_config, this, _1),
          freq_config_, event_sub_.last_tick());
  }
  return true;
}

bool flexran::app::log::elastic_search::set_batch_stats_max_size(int size)
{
  if (size < 1 || size > 1000) return false;
  batch_stats_max_no_ = size;
  return true;
}

bool flexran::app::log::elastic_search::set_batch_config_max_size(int size)
{
  if (size < 1 || size > 1000) return false;
  batch_config_max_no_ = size;
  return true;
}

void flexran::app::log::elastic_search::initialise_batch_stats()
{
  int ue_count = 0;
  for (uint64_t bs_id : rib_.get_available_base_stations()) {
    std::shared_ptr<rib::enb_rib_info> bs_config = rib_.get_bs(bs_id);
    ue_count += bs_config->get_ue_configs().ue_config().size();
  }

  batch_stats_current_no_ = 0;
  batch_stats_data_.clear();
  /* estimate size of stats:
   * (size of 1 UE's stats + 10%) * no of currently connected UEs * batch size
   * = (1250 * 1.1) * ue_count */
  batch_stats_data_.reserve(1400 * ue_count * batch_stats_max_no_);
}

void flexran::app::log::elastic_search::initialise_batch_config()
{
  const int bs_count = rib_.get_available_base_stations().size();

  batch_config_current_no_ = 0;
  batch_config_data_.clear();
  /* estimate size of config:
   * (size of 1 BS's stats + 10%) * no of currently connected BSs * batch size
   * = (2150 * 1.1) * ue_count */
  batch_config_data_.reserve(2400 * bs_count * batch_config_max_no_);
}

bool flexran::app::log::elastic_search::enable_logging()
{
  if (is_active())
    return false;
  if (freq_config_ == 0 && freq_stats_ == 0)
    return false;

  /* TODO check that curl connection succeeds? */

  active_since_ = std::chrono::system_clock::now();
  sent_packets_ = 0;
  initialise_batch_stats();
  initialise_batch_config();

  if (freq_config_ > 0) {
    tick_config_ = event_sub_.subscribe_task_tick(
        boost::bind(&flexran::app::log::elastic_search::process_config, this, _1),
        freq_config_, event_sub_.last_tick());
  }
  if (freq_stats_ > 0) {
    tick_stats_ = event_sub_.subscribe_task_tick(
        boost::bind(&flexran::app::log::elastic_search::process_stats, this, _1),
        freq_stats_, event_sub_.last_tick());
    /* UE disconnect: send batch if last UE disconnected */
    ue_disconnect_ = event_sub_.subscribe_ue_disconnect(
        boost::bind(&flexran::app::log::elastic_search::ue_disconnect, this, _1, _2));
  }
  tick_curl_ = event_sub_.subscribe_task_tick(
      boost::bind(&flexran::app::log::elastic_search::process_curl, this, _1),
        20, 0);

  return true;
}

bool flexran::app::log::elastic_search::disable_logging()
{
  if (tick_config_.connected()) tick_config_.disconnect();
  if (tick_stats_.connected()) tick_stats_.disconnect();
  if (ue_disconnect_.connected()) ue_disconnect_.disconnect();
  if (tick_curl_.connected()) tick_curl_.disconnect();
  wait_curl_end();
  return true;
}

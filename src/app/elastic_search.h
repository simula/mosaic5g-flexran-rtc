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

#ifndef ELASTIC_SEARCH_H_
#define ELASTIC_SEARCH_H_

#include "component.h"
#include "rib_common.h"

#include <curl/curl.h>
#include <vector>
#include <chrono>

namespace flexran {
  namespace app {
    namespace log {

      class elastic_search : public component {

      public:

        elastic_search(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub);
        ~elastic_search();

        bool add_endpoint(const std::string& ep);
        bool remove_endpoint(const std::string& ep);
        std::vector<std::string> get_endpoint() const { return elastic_search_ep_; }

        std::chrono::system_clock::time_point get_active_since() const
            { return active_since_; }
        int get_sent_packets() const { return sent_packets_; }
        bool set_freq_stats(int freq);
        int get_freq_stats() const { return freq_stats_; }
        bool set_freq_config(int freq);
        int get_freq_config() const { return freq_config_; }
        bool set_batch_stats_max_size(int size);
        int get_batch_stats_max_size() const { return batch_stats_max_no_; }
        bool set_batch_config_max_size(int size);
        int get_batch_config_max_size() const { return batch_config_max_no_; }

        bool enable_logging();
        bool disable_logging();
        bool is_active() const { return tick_config_.connected() || tick_stats_.connected(); }

      private:
        std::vector<std::string> elastic_search_ep_;

        std::chrono::system_clock::time_point active_since_;
        int sent_packets_;

        int freq_stats_;
        int freq_config_;

        int batch_stats_max_no_;
        int batch_stats_current_no_;
        std::string batch_stats_data_;
        void initialise_batch_config();
        void process_stats(uint64_t tick);
        void ue_disconnect(uint64_t bs_id, flexran::rib::rnti_t rnti);

        std::string bulk_create_index(const std::string& index , const std::string& data);

        int batch_config_max_no_;
        int batch_config_current_no_;
        std::string batch_config_data_;
        void initialise_batch_stats();
        void process_config(uint64_t tick);

        CURLM* curl_multi_;
        CURL* curl_create_transfer(const std::string& addr, const std::string& s);
        void curl_release_handles();

        void trigger_send(const std::string &s);
        void process_curl(uint64_t tick);
        void wait_curl_end();

        bs2::connection tick_stats_;
        bs2::connection ue_disconnect_;
        bs2::connection tick_config_;
        bs2::connection tick_curl_;
      };
    }
  }
}

#endif /* ELASTIC_SEARCH_H_ */

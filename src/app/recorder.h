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

/*! \file    recorder.h
 *  \brief   app for real-time statistics recording
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef _RECORDER_H_
#define _RECORDER_H_

#include <chrono>
#include <string>
#include <vector>
#include <map>

#include "component.h"
#include "rib_common.h"
#include "ue_mac_rib_info.h"

#include "flexran.pb.h"

using mac_harq_info_t = std::pair<protocol::flex_ue_stats_report,
                        std::array<bool, 8>>;

namespace flexran {

  namespace app {

    namespace log {

      class bs_dump {
      public:
        bs_dump(const protocol::flex_enb_config_reply& a,
                const protocol::flex_ue_config_reply& b,
                const protocol::flex_lc_config_reply& c,
                const std::vector<mac_harq_info_t>& d)
          : enb_config(a),
            ue_config(b),
            lc_config(c),
            ue_mac_harq_infos(d)
        { }
        protocol::flex_enb_config_reply                 enb_config;
        protocol::flex_ue_config_reply                  ue_config;
        protocol::flex_lc_config_reply                  lc_config;
        std::vector<mac_harq_info_t> ue_mac_harq_infos;
        bool operator==(const bs_dump& other) const;
      };

      enum job_type { all, enb, stats, bin };

      class job_info {
      public:
        job_info(uint64_t ms_start, uint64_t ms_end, const std::string& filename,
            job_type type)
          : ms_start(ms_start),
            ms_end(ms_end),
            filename(filename),
            type(type)
        { }
        uint64_t ms_start;
        uint64_t ms_end;
        std::string filename;
        job_type type;
      };

      class recorder : public component {

      public:

        recorder(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub),
            current_job_(nullptr)
        {}

        void tick(const bs2::connection& conn, uint64_t ms);
        bool start_meas(uint64_t duration, const std::string& type, std::string& id);
        bool get_job_info(const std::string& id, job_info& info);

        /**
         * method for serializing to JSON
         */
        static uint64_t write_json(job_info info,
            const std::vector<std::map<uint64_t, bs_dump>>& dump);

        /**
         * method for serializing to binary (custom)
         */
        static uint64_t write_binary(job_info info,
            const std::vector<std::map<uint64_t, bs_dump>>& dump);

        /**
         * methods for deserializing from binary (custom)
         */
        static std::vector<std::map<uint64_t, bs_dump>> read_binary(std::string filename);

      private:
        /* list of finished jobs that can be accessed via the NB REST API */
        std::vector<job_info> finished_jobs_;

        std::unique_ptr<std::vector<std::map<uint64_t, bs_dump>>> dump_;
        std::unique_ptr<job_info> current_job_;

        bs_dump record_chunk(uint64_t bs_id);

        /**
         * method for writer thread, passes to right (JSON/binary)
         * serialization method and moves job to writing_jobs_ after work.
         */
        void writer_method(std::unique_ptr<job_info> info,
            std::unique_ptr<std::vector<std::map<uint64_t, bs_dump>>> dump);

        static void write_json_chunk(std::ostream& s, job_type type,
            const std::map<uint64_t, bs_dump>& dump_chunk);
        static std::vector<std::string> get_ue_stats(
            const std::vector<mac_harq_info_t>& ue_mac_harq_infos);

        static void write_binary_chunk(std::ostream& s, const std::map<uint64_t, bs_dump>& dump_chunk);
        static void write_binary_ue_configs(std::ostream& s,
            const std::vector<mac_harq_info_t>& ue_mac_harq_infos);

        static std::map<uint64_t, bs_dump> read_binary_chunk(std::istream &s);
        static std::vector<mac_harq_info_t> read_binary_ue_configs(std::istream &s);

        /**
         * helper methods to write back protobuf messages
         */
        template <typename T>
        static bool write_flexran_message(std::ostream& s, const T& flex_message);
        template <typename T>
        static bool read_flexran_message(std::istream& s, T& flex_message);
      };
    }
  }
}

#endif /* _RECORDER_H_ */

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

#ifndef _RECORDER_H_
#define _RECORDER_H_

#include <chrono>
#include <string>
#include <vector>
#include <map>

#include "periodic_component.h"
#include "rib_common.h"
#include "ue_mac_rib_info.h"

#include "flexran.pb.h"

using mac_harq_info_t = std::pair<protocol::flex_ue_stats_report,
                        std::array<bool, 8>>;

namespace flexran {

  namespace app {

    namespace log {

      class agent_dump {
      public:
        agent_dump(const protocol::flex_enb_config_reply& a,
                  const protocol::flex_ue_config_reply& b,
                  const protocol::flex_lc_config_reply& c,
                  const std::map<flexran::rib::rnti_t, mac_harq_info_t>& d)
          : enb_config(a),
            ue_config(b),
            lc_config(c),
            ue_mac_harq_infos(d)
        { }
        protocol::flex_enb_config_reply                 enb_config;
        protocol::flex_ue_config_reply                  ue_config;
        protocol::flex_lc_config_reply                  lc_config;
        std::map<flexran::rib::rnti_t, mac_harq_info_t> ue_mac_harq_infos;
      };

      enum job_type { all, enb, stats, bin };

      class job_info {
      public:
        job_info(uint64_t ms_start, uint64_t ms_end, std::string& filename,
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

      class recorder : public periodic_component {

      public:

        recorder(rib::Rib& rib, const core::requests_manager& rm)
        : periodic_component(rib, rm),
          ms_counter_(0),
          current_job_(nullptr)
        {}

        void periodic_task();
        bool start_meas(uint64_t duration, const std::string& type, std::string& id);
        bool get_job_info(const std::string& id, job_info& info);

        /**
         * method for serializing to JSON
         */
        static void write_json(job_info info,
            const std::vector<std::map<int, agent_dump>>& dump);

        /**
         * method for serializing to binary (custom)
         */
        static void write_binary(job_info info,
            const std::vector<std::map<int, agent_dump>>& dump);

        /**
         * methods for deserializing from binary (custom)
         */
        static std::vector<std::map<int, agent_dump>> read_binary(std::string filename);

      private:
        uint64_t ms_counter_;

        /* list of finished jobs that can be accessed via the NB REST API */
        std::vector<job_info> finished_jobs_;

        std::unique_ptr<std::vector<std::map<int, agent_dump>>> dump_;
        std::unique_ptr<job_info> current_job_;

        agent_dump record_chunk(int agent_id);

        /**
         * method for writer thread, passes to right (JSON/binary)
         * serialization method and moves job to writing_jobs_ after work.
         */
        void writer_method(std::unique_ptr<job_info> info,
            std::unique_ptr<std::vector<std::map<int, agent_dump>>> dump);

        static void write_json_chunk(std::ostream& s, job_type type,
            const std::map<int, agent_dump>& dump_chunk);
        static void write_json_ue_configs(std::ostream& s,
            const std::map<flexran::rib::rnti_t, mac_harq_info_t>& ue_mac_harq_infos);

        static void write_binary_chunk(std::ostream& s, const std::map<int, agent_dump>& dump_chunk);
        static void write_binary_ue_configs(std::ostream& s,
            const std::map<flexran::rib::rnti_t, mac_harq_info_t>& ue_mac_harq_infos);

        static std::map<int, agent_dump> read_binary_chunk(std::istream &s);
        static std::map<flexran::rib::rnti_t, mac_harq_info_t> read_binary_ue_configs(std::istream &s);

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

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

/*! \file    agent_info.h
 *  \brief   agent information; agent(s) constitute a base station
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef AGENT_INFO_H_
#define AGENT_INFO_H_

#include "flexran.pb.h"
#include <vector>
#include <string>
#include <atomic>

namespace flexran {
  namespace rib {

    class agent_capabilities {
    public:
      agent_capabilities(const agent_capabilities& cap)
        : caps_(cap.caps_) {}
      agent_capabilities(const ::google::protobuf::RepeatedField<int>& proto_caps);

      bool orthogonal(const agent_capabilities& other) const;
      void merge_in(const agent_capabilities& other);
      bool is_complete() const;
      std::string to_string() const;
      std::string to_json() const;
      std::size_t size() const { return caps_.size(); }

    private:
      static uint32_t to_u32(const std::vector<protocol::flex_bs_capability> caps);
      static std::vector<protocol::flex_bs_capability> from_u32(uint32_t u);
      static std::vector<protocol::flex_bs_capability> make_caps(
          const ::google::protobuf::RepeatedField<int>& proto_caps);

      std::vector<protocol::flex_bs_capability> caps_;
    };

    class agent_info {
    public:
      agent_info(int agent_id, uint64_t bs_id, const agent_capabilities& cap,
                 const std::string &port_ip)
        : agent_id(agent_id),
          bs_id(bs_id),
          capabilities(cap),
          port_ip(port_ip),
          rx_packets(0),
          rx_bytes(0)
      { }
      std::string to_json() const;
      std::string to_string() const;

      const int agent_id;
      const uint64_t bs_id;
      const agent_capabilities capabilities;
      const std::string port_ip;
      std::atomic<uint64_t> rx_packets;
      std::atomic<uint64_t> rx_bytes;
    };
  }
}

#endif /* AGENT_INFO_H_ */

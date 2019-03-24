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

/*! \file    agent_info.cc
 *  \brief   agent information; agent(s) constitute a base station
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include "agent_info.h"
#include <google/protobuf/util/json_util.h>
#include <stdexcept>
#include <algorithm>

flexran::rib::agent_capabilities::agent_capabilities(
    const ::google::protobuf::RepeatedField<int>& proto_caps)
{
  if (proto_caps.size() < 1)
    throw std::runtime_error("illegal: zero capabilities");
  // this sorts the array and saves it
  caps_ = from_u32(to_u32(make_caps(proto_caps)));
}

bool flexran::rib::agent_capabilities::orthogonal(const agent_capabilities& other) const
{
  return (to_u32(caps_) & to_u32(other.caps_)) == 0;
}

void flexran::rib::agent_capabilities::merge_in(
    const agent_capabilities& other)
{
  caps_ = from_u32(to_u32(caps_) | to_u32(other.caps_));
}

bool flexran::rib::agent_capabilities::is_complete() const
{
  // all eight bits are set for LOPHY, HIPHY, LOMAC, HIMAC, RLC, PDCP, SDAP, RRC,
  // and possibly S1AP (to support older versions)
  return (to_u32(caps_) & 0xFF) == 0xFF;
}

std::string flexran::rib::agent_capabilities::to_string() const
{
  std::string s = "[";
  for (protocol::flex_bs_capability c: caps_)
    s += protocol::flex_bs_capability_Name(c) + ",";
  s[s.size() - 1] = ']';
  return s;
}

std::string flexran::rib::agent_capabilities::to_json() const
{
  std::string s = "[";
  for (protocol::flex_bs_capability c: caps_)
    s += "\"" + protocol::flex_bs_capability_Name(c) + "\",";
  s[s.size() - 1] = ']';
  return s;
}

uint32_t flexran::rib::agent_capabilities::to_u32(
    const std::vector<protocol::flex_bs_capability> caps)
{
  uint32_t u = 0;
  for (protocol::flex_bs_capability c: caps)
    u |= 1 << (static_cast<int>(c));
  return u;
}

std::vector<protocol::flex_bs_capability>
flexran::rib::agent_capabilities::from_u32(uint32_t u)
{
  std::vector<protocol::flex_bs_capability> caps;
  uint32_t n = 1;
  while (u > 0) {
    if ((u & 0x1) == 1)
      caps.push_back(static_cast<protocol::flex_bs_capability>(n - 1));
    n += 1;
    u >>= 1;
  }
  return caps;
}

std::vector<protocol::flex_bs_capability>
flexran::rib::agent_capabilities::make_caps(
    const ::google::protobuf::RepeatedField<int>& proto_caps)
{
  std::vector<protocol::flex_bs_capability> cs;
  for (int i: proto_caps)
    cs.push_back(static_cast<protocol::flex_bs_capability>(i));
  return cs;
}

flexran::rib::agent_splits::agent_splits(
    const ::google::protobuf::RepeatedField<int>& proto_splits)
{
  std::transform(proto_splits.begin(), proto_splits.end(),
      std::back_inserter(splits_),
      [] (int p) { return static_cast<protocol::flex_bs_split>(p); });
}

std::string flexran::rib::agent_splits::to_string() const
{
  if (splits_.size() < 1) return "[]";
  std::string s = "[";
  for (protocol::flex_bs_split sp: splits_)
    s += protocol::flex_bs_split_Name(sp) + ",";
  s[s.size() - 1] = ']';
  return s;
}

std::string flexran::rib::agent_splits::to_json() const
{
  if (splits_.size() < 1) return "[]";
  std::string s = "[";
  for (protocol::flex_bs_split sp: splits_)
    s += "\"" + protocol::flex_bs_split_Name(sp) + "\",";
  s[s.size() - 1] = ']';
  return s;
}

std::string flexran::rib::agent_info::to_json() const
{
  std::string s = "{";
  s += "\"agent_id\":" + std::to_string(agent_id);
  s += ",\"ip_port\":\"" + port_ip;
  s += "\",\"bs_id\":" + std::to_string(bs_id);
  s += ",\"capabilities\":" + capabilities.to_json();
  s += ",\"splits\":" + splits.to_json();
  s += "}";
  return s;
}

std::string flexran::rib::agent_info::to_string() const
{
  std::string s;
  s += "agent ID " + std::to_string(agent_id);
  s += " (" + port_ip + ")";
  s += " for BS " + std::to_string(bs_id);
  s += ", capabilities: " + capabilities.to_string();
  s += ", splits: " + splits.to_string();
  return s;
}

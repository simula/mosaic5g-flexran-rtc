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

/*! \file    protocol_message.cc
 *  \brief   helper for handling protobuf messages
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include <cstring>
#include <cstdlib>

#include "flexran_log.h"
#include "protocol_message.h"

flexran::network::protocol_message::protocol_message(protocol_message&& other)
  : dynamic_alloc_(other.dynamic_alloc_),
    body_length_(other.body_length_)
{
  /* to improve efficiency: disable copy constructor and only move this
   * protocol message. Disable dynamic_alloc_ in other so we don't free twice.
   * I hope I got it right... */
  other.dynamic_alloc_ = false;
  if (dynamic_alloc_) {
    data_ = other.data_;
  } else {
    data_ = stat_data_;
    std::memcpy(data_, other.stat_data_, header_length + body_length_);
  }
}

void flexran::network::protocol_message::set_body_length(std::size_t new_length) {
  if (dynamic_alloc_)
    delete [] data_;
  if (new_length > max_normal_body_length) {
    data_ = new unsigned char[header_length + new_length];
    dynamic_alloc_ = true;
  } else {
    data_ = stat_data_;
    dynamic_alloc_ = false;
  }
  body_length_ = new_length;
}

void flexran::network::protocol_message::set_message(const char * buf, std::size_t size) {
  set_body_length(size);
  encode_header(size);
  std::memcpy(data_ + header_length, buf, size);
}

bool flexran::network::protocol_message::decode_header() {
  int len = (data_[0] << 24) | (data_[1] << 16) | (data_[2] << 8) | data_[3];
  set_body_length(len);
  encode_header(len); /* re-encode header since it might be lost due to reallocation */
  return true;
}

void flexran::network::protocol_message::encode_header(std::size_t size) {
  data_[0] = (size >> 24) & 255;
  data_[1] = (size >> 16) & 255;
  data_[2] = (size >> 8) & 255;
  data_[3] = size & 255;
}

flexran::network::protocol_message::~protocol_message() {
  if (dynamic_alloc_)
    delete [] data_;
}

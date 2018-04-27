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
#include <iostream>

#include "protocol_message.h"

void flexran::network::protocol_message::body_length(std::size_t new_length) {
  body_length_ = new_length;
  if (body_length_ > max_body_length)
    body_length_ = max_body_length;
}

void flexran::network::protocol_message::set_message(const char * buf, std::size_t size) {
  body_length(size);
  encode_header();
  std::memcpy(data_ + header_length, buf, body_length_);
}

bool flexran::network::protocol_message::decode_header() {
  body_length_ = (data_[0] << 24) |
    (data_[1] << 16) |
    (data_[2] << 8)  |
    data_[3];
  
  if (body_length_ > max_body_length) {
    body_length_ = 0;
    return false;
  }
  return true;
}

void flexran::network::protocol_message::encode_header() {
  data_[0] = (body_length_ >> 24) & 255;
  data_[1] = (body_length_ >> 16) & 255;
  data_[2] = (body_length_ >> 8) & 255;
  data_[3] = body_length_ & 255;
}

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

/*! \file    tagged_message.cc
 *  \brief   represent a protobuf message with a tag
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include "tagged_message.h"

flexran::network::tagged_message::tagged_message(const char * msg, std::size_t size, int tag):
  size_(size), tag_(tag) {
  if (size <= max_normal_msg_size) {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  } else {
    msg_contents_ = new char[size];
    dynamic_alloc_ = true;
  }
  std::memcpy(msg_contents_, msg, size);
}

flexran::network::tagged_message::tagged_message(std::size_t size, int tag):
  size_(size), tag_(tag) {
  if (size > max_normal_msg_size) {
    msg_contents_ = new char[size];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
}
  
flexran::network::tagged_message::tagged_message(const tagged_message& m) {
  tag_ = m.getTag();
  size_ = m.getSize();
  if (size_ > max_normal_msg_size) {
    msg_contents_ = new char[size_];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
  std::memcpy(msg_contents_, m.getMessageContents(), size_);
}

flexran::network::tagged_message::tagged_message(tagged_message&& other) {
  tag_ = other.getTag();
  size_ = other.getSize();

  if (size_ > max_normal_msg_size) {
    msg_contents_ = new char[size_];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
  std::memcpy(msg_contents_, other.getMessageContents(), size_);
}
  
flexran::network::tagged_message& flexran::network::tagged_message::operator=(flexran::network::tagged_message&& other) {
  if (dynamic_alloc_) {
    delete [] msg_contents_;
  }

  tag_ = other.getTag();
  size_ = other.getSize();
  if (size_ > max_normal_msg_size) {
    msg_contents_ = new char[size_];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
  std::memcpy(msg_contents_, other.getMessageContents(), size_);
  return *this;
}

flexran::network::tagged_message::~tagged_message() {
  if (dynamic_alloc_) {
    delete [] msg_contents_;
  }
}

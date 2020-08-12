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

/*! \file    protocol_message.h
 *  \brief   helper for handling protobuf messages
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef PROTOCOL_MESSAGE_H_
#define PROTOCOL_MESSAGE_H_

#include "flexran.pb.h"

namespace flexran {

  namespace network {
    
    class protocol_message {
      
    public:
      enum { header_length = 4 };
      enum { max_normal_body_length = 24000 };
      
      protocol_message()
        : data_(stat_data_),
          dynamic_alloc_(false),
          body_length_(0) {}
      protocol_message(const char *buf, std::size_t size);
      protocol_message(const protocol_message&) = delete;
      protocol_message(protocol_message&& s);
      
      const char* data() const {
	return (char *) data_;
      }
      
      char* data() {
	return (char *) data_;
      }
      
      uint32_t length() const {
	return header_length + body_length_;
      }
      
      const char* body() const {
	return (char *) data_ + header_length;
      }
      
      char* body() {
	return (char *) data_ + header_length;
      }
      
      std::size_t body_length() const {
	return body_length_;
      }
      
      void set_body_length(std::size_t new_length);
      
      void set_message(const char * buf, std::size_t size);
      
      bool decode_header();
      
      void encode_header(std::size_t size);

      ~protocol_message();
      
    private:
      unsigned char *data_;
      unsigned char stat_data_[header_length + max_normal_body_length];
      bool dynamic_alloc_;
      uint32_t body_length_;
      
    };
    
  }
}

#endif

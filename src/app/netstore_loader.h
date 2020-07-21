/*
 * Copyright 2016-2020 FlexRAN Authors, Eurecom and The University of Edinburgh
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

/*! \file    netstore_loader.h
 *  \brief   NetStore downloading app
 *  \authors Robert Schmidt, Firas Abdeljelil
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr, firas.abdeljelil@eurecom.fr
 */

#ifndef NETSTORE_LOADER_H_
#define NETSTORE_LOADER_H_

#include "component.h"
#include "rib_common.h"
#include <curl/curl.h>

namespace flexran {

  namespace app {

    namespace management {

      class netstore_loader : public component {

      public:
       netstore_loader(const rib::Rib &rib,
                       const core::requests_manager &rm,
                       event::subscription &sub);
       ~netstore_loader();
       bool add_endpoint(const std::string& ep);

       CURLM* curl_multi_;
       CURL* curl_create_transfer(const std::string& addr);

       void trigger_app_request(const std::string& id);
       void trigger_app_stop(const std::string& id);
       void trigger_app_reconfig(const std::string &id, const std::string& policy);

      private:
       bs2::connection tick_list_;
       bs2::connection tick_retrieve_;

       void process_list(uint64_t tick,
                         const std::string& id,
                         uint32_t xid,
                         protocol::flex_control_delegation_type type);
       void process_retrieve(uint64_t tick,
                             const std::string& id,
                             uint32_t xid,
                             protocol::flex_control_delegation_type type);
       void push_code(uint64_t bs_id,
                      uint32_t xid,
                      std::string object_name,
                      const char* data,
                      size_t len,
                      protocol::flex_control_delegation_type type);
       void send_rest_request(const std::string& addr);
       bool check_list(const std::string& id);

       void push_app_reconfiguration(uint64_t bs_id,
            std::string object_name,
            std::string action,
            const ::google::protobuf::Map<std::string, protocol::flex_agent_reconfiguration_param > *params);
      };
    }
  }
}

#endif /* NETSTORE_LOADER_H_ */

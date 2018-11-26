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

/*! \file    rib_updater.h
 *  \brief   dispatcher of protobuf messages, maintains RIB
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef RIB_UPDATER_H_
#define RIB_UPDATER_H_

#include "async_xface.h"
#include "requests_manager.h"
#include "rib.h"
#include "flexran.pb.h"
#include "rt_task.h"
#include "subscription.h"
#include <chrono>

namespace flexran {

  namespace rib {

    class rib_updater {

    public:
    rib_updater(Rib& storage, flexran::network::async_xface& xface,
        flexran::core::requests_manager& netman,
        flexran::event::subscription& ev, int n_msg_check = 350)
      : rib_(storage), net_xface_(xface), req_manager_(netman),
        event_sub_(ev), messages_to_check_(n_msg_check) {}
      
      unsigned int run();
      
      unsigned int update_rib();

#ifdef PROFILE
      void print_prof_results(std::chrono::duration<double> d);
#endif

    private:
      
      // Incoming message handlers
      void handle_new_connection(int agent_id);
      void dispatch_message(std::shared_ptr<flexran::network::tagged_message> tm);

      void handle_hello(int agent_id,
          const protocol::flex_hello& hello_msg,
          protocol::flexran_direction dir);

      void handle_echo_request(int agent_id,
          const protocol::flex_echo_request& echo_request_msg);

      void handle_echo_reply(int agent_id,
          const protocol::flex_echo_reply& echo_reply_msg);

      void handle_sf_trigger(int agent_id,
          const protocol::flex_sf_trigger& sf_trigger_msg);

      void handle_enb_config_reply(int agent_id,
          const protocol::flex_enb_config_reply& enb_config_reply_msg);

      void handle_ue_config_reply(int agent_id,
          const protocol::flex_ue_config_reply& ue_config_reply_msg);

      void handle_lc_config_reply(int agent_id,
          const protocol::flex_lc_config_reply& lc_config_reply_msg);

      void handle_stats_reply(int agent_id,
          const protocol::flex_stats_reply& mac_stats_reply);

      void handle_ue_state_change(int agent_id,
          const protocol::flex_ue_state_change& ue_state_change_msg);

      void handle_disconnect(int agent_id,
          const protocol::flex_disconnect& disconnect_msg);

      void trigger_bs_config(uint64_t bs_id);

      void warn_unknown_agent_bs(const std::string& function, int agent_id);
      
      Rib& rib_;
      // RX, can TX to individual agents
      flexran::network::async_xface& net_xface_;
      // TX to base station, possibly inferring the right agent
      flexran::core::requests_manager& req_manager_;
      // Event subscription system informing apps
      flexran::event::subscription& event_sub_;
      
      // Max number of messages to check during a single update period
      std::atomic<int> messages_to_check_;
      static constexpr const uint64_t BS_ID_OFFSET = 10000;
      
    };

  }

}

#endif

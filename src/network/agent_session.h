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

/*! \file    agent_session.h
 *  \brief   represents the session of an agent at the controller
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef AGENT_SESSION_H_
#define AGENT_SESSION_H_

#include <deque>
#include <boost/asio.hpp>

#include "flexran.pb.h"
#include "connection_manager.h"
#include "protocol_message.h"
#include "async_xface.h"

namespace flexran {

  namespace network {
  
    typedef std::deque<protocol_message> flexran_protocol_queue;

    class async_xface;
    class connection_manager;
    class agent_session :
      public std::enable_shared_from_this<agent_session> {
      
    public:
    agent_session(boost::asio::ip::tcp::socket socket,
		  connection_manager& manager,
		  async_xface& xface,
		  int session_id)
      : socket_(std::move(socket)), session_id_(session_id), manager_(manager), xface_(xface),
        ip_port_(socket_.remote_endpoint().address().to_string() + ":" + std::to_string(socket_.remote_endpoint().port())) {
	socket_.set_option(boost::asio::ip::tcp::no_delay(true));
      }
      
      void start();

      void deliver(std::shared_ptr<tagged_message> msg);
      void close();
      std::string get_endpoint() const { return ip_port_; }
      
    private:
      
      void do_read_header();
      void do_read_body();
      void do_write();
      void generate_disconnect_msg();
      
      boost::asio::ip::tcp::socket socket_;
      flexran_protocol_queue write_queue_;
      
      
      protocol_message read_msg_;
      int session_id_;
      connection_manager& manager_;
      async_xface& xface_;

      const std::string ip_port_;
    };
  }
}
#endif

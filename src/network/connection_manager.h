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

/*! \file    connection_manager.h
 *  \brief   handles sessions of agents
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <boost/asio.hpp>
#include <unordered_map>

#include "agent_session.h"
#include "async_xface.h"

namespace flexran {

  namespace network {

    class async_xface;
    class agent_session;
    class connection_manager:
      public std::enable_shared_from_this<connection_manager> {
      
    public:
      connection_manager(boost::asio::io_service & io_service,
			 const boost::asio::ip::tcp::endpoint& endpoint,
			 async_xface& xface);
      
      void close_connection(int session_id);
      
      void send_msg_to_agent(std::shared_ptr<tagged_message> msg);
      std::string get_endpoint(int session_id);
      
    private:
      
      void do_accept();
      
      boost::asio::ip::tcp::acceptor acceptor_;
      boost::asio::ip::tcp::socket socket_;
      
      std::unordered_map<int, std::shared_ptr<agent_session>> sessions_;
      int next_id_;
      async_xface& xface_;
    };

  }
  
}   

#endif

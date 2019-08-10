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

/*! \file    async_xface.h
 *  \brief   asynchronous message exchange
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef ASYNC_XFACE_H_
#define ASYNC_XFACE_H_

#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>

#include "tagged_message.h"
#include "connection_manager.h"
#include "rt_task.h"

namespace flexran {

  namespace network {
    
    class connection_manager;
    class async_xface : public flexran::core::rt::rt_task {
    public:
    async_xface(int port): rt_task(Policy::FIFO, 60),
        endpoint_(boost::asio::ip::tcp::v4(), port), port_(port)  {}
      
      void run();
      void end();
      
      void establish_xface();
      
      void forward_message(tagged_message *msg);
      
      bool get_msg_from_network(std::shared_ptr<tagged_message>& msg);
      
      bool send_msg(const protocol::flexran_message& msg, int agent_tag) const;
      std::string get_endpoint(int agent_id) const;
      
      void forward_msg_to_agent();

      void initialize_connection(int session_id);
      void release_connection(int session_id);
      
    private:
      
      mutable boost::asio::io_service io_service;
      std::unique_ptr<boost::asio::io_service::work> work_ptr_;

      mutable boost::lockfree::queue<tagged_message *, boost::lockfree::fixed_sized<true>> in_queue_{10000};
      mutable boost::lockfree::queue<tagged_message *, boost::lockfree::fixed_sized<true>> out_queue_{10000};

      mutable boost::asio::ip::tcp::endpoint endpoint_;

      std::unique_ptr<connection_manager> manager_;
  
      int port_;

      mutable async_xface* self_ = this;   
    };

  }

}

#endif

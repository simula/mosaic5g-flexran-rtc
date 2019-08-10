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

/*! \file    async_xface.cc
 *  \brief   asynchronous message exchange
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include <boost/bind.hpp>

#include "async_xface.h"
#include "rt_wrapper.h"

void flexran::network::async_xface::run() {
  establish_xface();
}

void flexran::network::async_xface::end(){
  io_service.stop();
}

void flexran::network::async_xface::establish_xface() {
  manager_.reset(new connection_manager(io_service, endpoint_, *this));
  work_ptr_.reset(new boost::asio::io_service::work(io_service));
  io_service.run();
}

void flexran::network::async_xface::forward_message(tagged_message *msg) {
  in_queue_.push(msg);
}

bool flexran::network::async_xface::get_msg_from_network(std::shared_ptr<tagged_message>& msg) {
  return in_queue_.consume_one([&] (tagged_message *tm) {
      std::shared_ptr<tagged_message> p(std::move(tm));
      msg = p;});
}

bool flexran::network::async_xface::send_msg(const protocol::flexran_message& msg, int agent_tag) const {
  tagged_message *tm =  new tagged_message(msg.ByteSize(), agent_tag);
  msg.SerializeToArray(tm->getMessageArray(), msg.ByteSize());
  if (out_queue_.push(tm)) {
    io_service.post(boost::bind(&async_xface::forward_msg_to_agent, self_));
    return true;
  } else {
    return false;
  }
}

std::string flexran::network::async_xface::get_endpoint(int agent_id) const
{
  return manager_->get_endpoint(agent_id);
}

void flexran::network::async_xface::forward_msg_to_agent() {
  tagged_message *msg;
  out_queue_.pop(msg);
  std::shared_ptr<tagged_message> message(msg);
  manager_->send_msg_to_agent(message);
}


void flexran::network::async_xface::initialize_connection(int session_id) {
  tagged_message *th = new tagged_message(0, session_id);
  in_queue_.push(th);
}

void flexran::network::async_xface::release_connection(int session_id)
{
  manager_->close_connection(session_id);
}

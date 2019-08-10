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

/*! \file    connection_manager.cc
 *  \brief   handles sessions of agents
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include "connection_manager.h"
#include "flexran_log.h"

flexran::network::connection_manager::connection_manager(boost::asio::io_service& io_service,
				       const boost::asio::ip::tcp::endpoint& endpoint,
				       async_xface& xface)
  : acceptor_(io_service, endpoint), socket_(io_service), next_id_(0), xface_(xface) {

  do_accept();

}

void flexran::network::connection_manager::send_msg_to_agent(std::shared_ptr<tagged_message> msg) {
  auto it = sessions_.find(msg->getTag());
  if (it == sessions_.end()) {
    LOG4CXX_WARN(flog::net, "Message for non-existent session " << msg->getTag() << " discarded");
    return;
  }
  sessions_[msg->getTag()]->deliver(msg);
}

void flexran::network::connection_manager::do_accept() {

  acceptor_.async_accept(socket_,
			 [this](boost::system::error_code ec) {
      if (!ec) {
	sessions_[next_id_] = std::make_shared<agent_session>(std::move(socket_), *this, xface_, next_id_);
	sessions_[next_id_]->start();
	xface_.initialize_connection(next_id_);
	next_id_++;
      }
      
      do_accept();
			 });
}

void flexran::network::connection_manager::close_connection(int session_id) {
  auto it = sessions_.find(session_id);
  if (it != sessions_.end()) {
    it->second->close();
    sessions_.erase(session_id);
  }
}

std::string flexran::network::connection_manager::get_endpoint(int session_id) {
  auto it = sessions_.find(session_id);
  if (it == sessions_.end())
    return "";
  return it->second->get_endpoint();
}

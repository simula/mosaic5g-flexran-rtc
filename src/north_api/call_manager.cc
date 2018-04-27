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

/*! \file    call_manager.cc
 *  \brief   manager for HTTP server
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#include "call_manager.h"

#include <iostream>


void flexran::north_api::manager::call_manager::init(size_t thr) {
  auto opts = Pistache::Http::Endpoint::options().threads(thr)
      .flags(Pistache::Tcp::Options::ReuseAddr);
  httpEndpoint->init(opts);
}

void flexran::north_api::manager::call_manager::start() {
  httpEndpoint->setHandler(router_.handler());
  httpEndpoint->serveThreaded();
}

void flexran::north_api::manager::call_manager::shutdown() {
  httpEndpoint->shutdown();
}

void flexran::north_api::manager::call_manager::register_calls(flexran::north_api::app_calls& calls) {

  calls.register_calls(router_);
  
}

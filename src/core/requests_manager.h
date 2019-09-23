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

/*! \file    requests_manager.h
 *  \brief   bridge between the apps and the network interface
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef REQUESTS_MANAGER_H_
#define REQUESTS_MANAGER_H_

#include "flexran.pb.h"

namespace flexran {

  namespace rib {
    class Rib;
  }
  namespace network {
    class async_xface;
  }

  namespace core {

    class requests_manager {

    public:
      requests_manager(flexran::rib::Rib& rib, flexran::network::async_xface& xface)
        : rib_(rib), net_xface_(xface) {}
      
      void send_message(uint64_t bs_id, const protocol::flexran_message& msg) const;
      
    private:
      const flexran::rib::Rib& rib_;
      flexran::network::async_xface& net_xface_;
      
    };

  }

}

#endif

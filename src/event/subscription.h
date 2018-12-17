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

/*! \file    subscription.h
 *  \brief   Event subscription: callback subscription for certain events
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef SUBSCRIPTION_H_RRRR
#define SUBSCRIPTION_H_RRRR

#include <atomic>
#include <boost/signals2.hpp>
namespace bs2 = boost::signals2;

#include "callbacks.h"

namespace flexran {
  namespace rib {
    class rib_updater;
  }
  namespace core {
    class task_manager;
  }
}

namespace flexran {
  namespace event {
    class subscription {
    public:
      // friend classes can access private fields
      friend class flexran::rib::rib_updater;
      friend class flexran::core::task_manager;

      subscription() : last_tick_(0) {}
      uint64_t last_tick() const { return last_tick_; }

      // in the following, functions without _extended subscribe to "simple"
      // callback (as in callbacks.h), while the _extended versions subscribe
      // to a callback passing the connection which can be used to disconnect.
      // In both cases, they return the actual connection which can also be
      // used to disconnect.
      bs2::connection subscribe_bs_add(const bs_cb::slot_type& cb);
      bs2::connection subscribe_bs_add_extended(const bs_cb::extended_slot_type& cb);
      bs2::connection subscribe_bs_remove(const bs_cb::slot_type& cb);
      bs2::connection subscribe_bs_remove_extended(const bs_cb::extended_slot_type& cb);

      bs2::connection subscribe_ue_connect(const ue_cb::slot_type& cb);
      bs2::connection subscribe_ue_connect_extended(const ue_cb::extended_slot_type& cb);
      bs2::connection subscribe_ue_update(const ue_cb::slot_type& cb);
      bs2::connection subscribe_ue_update_extended(const ue_cb::extended_slot_type& cb);
      bs2::connection subscribe_ue_disconnect(const ue_cb::slot_type& cb);
      bs2::connection subscribe_ue_disconnect_extended(const ue_cb::extended_slot_type& cb);

      bs2::connection subscribe_task_tick(const task_cb::slot_type& cb,
          uint64_t period, uint64_t start = 0);
      bs2::connection subscribe_task_tick_extended(const task_cb::extended_slot_type& cb,
          uint64_t period, uint64_t start = 0);
      
    private:
      bs_cb bs_add_;
      bs_cb bs_remove_;

      ue_cb ue_connect_;
      ue_cb ue_update_;
      ue_cb ue_disconnect_;

      task_cb task_tick_;
      std::atomic<uint64_t> last_tick_; // used to calculate offsets
    };
  }
}

#endif /* SUBSCRIPTION_H_ */

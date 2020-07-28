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

/*! \file    callbacks.h
 *  \brief   Callback definitions for controller events
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include <boost/signals2.hpp>
namespace bs2 = boost::signals2;
#include "rib_common.h"
#include "flexran.pb.h"

namespace flexran {
  namespace event {
    /// Single-thread callback for BS events (add, remove)
    /// Argument is BS ID
    typedef bs2::signal_type<void(uint64_t),
        bs2::keywords::mutex_type<bs2::dummy_mutex>>::type bs_cb;

    /// Single-thread callback for UE events (connect, update, disconnect)
    /// Argument is BS ID and RNTI
    typedef bs2::signal_type<void(uint64_t, flexran::rib::rnti_t),
        bs2::keywords::mutex_type<bs2::dummy_mutex>>::type ue_cb;

    /// Single-thread callback for Task events (tick)
    /// Argument is current task iteration (on a ms basis, but might be
    /// inaccurate due to skipped milliseconds)
    typedef bs2::signal_type<void(uint64_t),
        bs2::keywords::mutex_type<bs2::dummy_mutex>>::type task_cb;


    /// Single-thread callback for arbitrary protobuf message
    /// Argument is BS ID and the actual message
    template <typename ProtobufMsg>
    using msg_cb = typename bs2::signal_type<void(uint64_t, ProtobufMsg),
        bs2::keywords::mutex_type<bs2::dummy_mutex>>::type;
  }
}

#endif /* CALLBACKS_H_ */

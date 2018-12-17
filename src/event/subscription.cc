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

/*! \file    subscription.cc
 *  \brief   Event subscription: callback subscription for certain events
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include "subscription.h"
#include <algorithm>

bs2::connection
flexran::event::subscription::subscribe_bs_add(const bs_cb::slot_type& cb)
{
  return bs_add_.connect(cb);
}

bs2::connection
flexran::event::subscription::subscribe_bs_add_extended(const bs_cb::extended_slot_type& cb)
{
  return bs_add_.connect_extended(cb);
}

bs2::connection
flexran::event::subscription::subscribe_bs_remove(const bs_cb::slot_type& cb)
{
  return bs_remove_.connect(cb);
}

bs2::connection
flexran::event::subscription::subscribe_bs_remove_extended(const bs_cb::extended_slot_type& cb)
{
  return bs_remove_.connect_extended(cb);
}

bs2::connection
flexran::event::subscription::subscribe_ue_connect(const ue_cb::slot_type& cb)
{
  return ue_connect_.connect(cb);
}

bs2::connection
flexran::event::subscription::subscribe_ue_connect_extended(const ue_cb::extended_slot_type& cb)
{
  return ue_connect_.connect_extended(cb);
}

bs2::connection
flexran::event::subscription::subscribe_ue_update(const ue_cb::slot_type& cb)
{
  return ue_update_.connect(cb);
}

bs2::connection
flexran::event::subscription::subscribe_ue_update_extended(const ue_cb::extended_slot_type& cb)
{
  return ue_update_.connect_extended(cb);
}

bs2::connection
flexran::event::subscription::subscribe_ue_disconnect(const ue_cb::slot_type& cb)
{
  return ue_disconnect_.connect(cb);
}

bs2::connection
flexran::event::subscription::subscribe_ue_disconnect_extended(const ue_cb::extended_slot_type& cb)
{
  return ue_disconnect_.connect_extended(cb);
}

bs2::connection
flexran::event::subscription::subscribe_task_tick(const task_cb::slot_type& cb,
    uint64_t period, uint64_t start)
{
  auto f = [period,start,cb] (uint64_t t)
           {
             if (t >= start && (t - start) % period == 0) cb(t);
           };
  return task_tick_.connect(f);
}

bs2::connection
flexran::event::subscription::subscribe_task_tick_extended(const task_cb::extended_slot_type& cb,
    uint64_t period, uint64_t start)
{
  auto f = [period,start,cb] (const bs2::connection& c, uint64_t t)
           {
             if (t >= start && (t - start) % period == 0) cb(c, t);
           };
  return task_tick_.connect_extended(f);
}

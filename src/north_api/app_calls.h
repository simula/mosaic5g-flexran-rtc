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

/*! \file    app_calls.h
 *  \brief   base class of FlexRAN NB API
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef APP_CALLS_H_
#define APP_CALLS_H_

#include <pistache/router.h>

namespace REQ_TYPE {
  constexpr const char *ALL_STATS  = "all";
  constexpr const char *ENB_CONFIG = "enb_config";
  constexpr const char *MAC_STATS  = "mac_stats";
}

namespace flexran {

  namespace north_api {

    class app_calls {

    public:

      virtual void register_calls(Pistache::Rest::Description& desc) = 0;
      static constexpr const size_t AGENT_ID_LENGTH_LIMIT = 3;
      static constexpr const size_t RNTI_ID_LENGTH_LIMIT  = 6;

    };
  }
}

#endif /* APP_CALLS_H_ */

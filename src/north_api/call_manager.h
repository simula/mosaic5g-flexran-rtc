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

/*! \file    call_manager.h
 *  \brief   manager for HTTP server
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef CALL_MANAGER_H_
#define CALL_MANAGER_H_

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"

namespace flexran {

  namespace north_api {

    namespace manager {

      class call_manager {

      public:

        call_manager(Pistache::Address addr);

	void init(size_t thr = 1);

	void start();

	void shutdown();

	void register_calls(flexran::north_api::app_calls& calls);
	
      private:

	void setup_routes();
        void list_api(const Pistache::Rest::Request& request,
                      Pistache::Http::ResponseWriter response);
	
	std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
        Pistache::Rest::Description desc_;
      };
      
    }
    
  }
  
}

#endif /* CALL_MANAGER_H_ */

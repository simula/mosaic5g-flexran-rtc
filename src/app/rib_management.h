/* The MIT License (MIT)

   Copyright (c) 2017 Robert Schmidt

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#ifndef RIB_MANAGEMENT_H_
#define RIB_MANAGEMENT_H_

#include <chrono>

#include "periodic_component.h"
#include "rib_common.h"

namespace flexran {

  namespace app {

    namespace management {

      class rib_management : public periodic_component {

      public:

        rib_management(rib::Rib& rib, const core::requests_manager& rm)
	  : periodic_component(rib, rm),
            ms_counter(1),
            last_now(std::chrono::steady_clock::now()) {}

        void run_periodic_task();

      private:
        int ms_counter;
        std::chrono::steady_clock::time_point last_now;
        std::set<int> inactive_agents;

        void send_enb_config_request(int agent_id);
      };
    }
  }
}

#endif /* RIB_MANAGEMENT_H_ */

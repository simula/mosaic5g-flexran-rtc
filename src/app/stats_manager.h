/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

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

#ifndef STATS_MANAGER_H_
#define STATS_MANAGER_H_

#include <set>

#include "periodic_component.h"

namespace flexran {

  namespace app {

    namespace stats {

      class stats_manager : public periodic_component {
	
      public:
	
      stats_manager(flexran::rib::Rib& rib, const flexran::core::requests_manager& rm)
      : periodic_component(rib, rm) {}
	
      void periodic_task();

      std::string all_stats_to_string() const;
      std::string all_stats_to_json_string() const;
      bool stats_by_enb_id_to_json_string(uint64_t enb_id, std::string& out) const;
      bool stats_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const;

      std::string all_enb_configs_to_string() const;
      std::string all_enb_configs_to_json_string() const;
      bool enb_configs_by_enb_id_to_json_string(uint64_t enb_id, std::string& out) const;
      bool enb_configs_by_agent_id_to_json_string(int agent_id, std::string& out) const;

      std::string all_mac_configs_to_string() const;
      std::string all_mac_configs_to_json_string() const;
      bool mac_configs_by_enb_id_to_json_string(uint64_t enb_id, std::string& out) const;
      bool mac_configs_by_agent_id_to_json_string(uint64_t agent_id, std::string& out) const;
	
	
      private:

        std::set<int> agent_list_;

      };

    }

  }

}

#endif

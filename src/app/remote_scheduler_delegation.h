#ifndef REMOTE_SCHEDULER_DELEGATION_H_
#define REMOTE_SCHEDULER_DELEGATION_H_

#include <map>

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

namespace progran {

  namespace app {

    namespace scheduler {

      class remote_scheduler_delegation : public periodic_component {
	
      public:
	
      remote_scheduler_delegation(const rib::Rib& rib, const core::requests_manager& rm)
	: periodic_component(rib, rm), delegation_enabled_(false) {}
	
	void run_periodic_task();
	
	static int32_t tpc_accumulated;
	
      private:
	
	::std::shared_ptr<enb_scheduling_info> get_scheduling_info(int agent_id);
	
	::std::map<int, ::std::shared_ptr<enb_scheduling_info>> scheduling_info_;
	
	::std::atomic<bool> delegation_enabled_;
	
	// Set these values internally for now
	
	const int schedule_ahead = 4;
	
      };

    }
    
  }

}

#endif
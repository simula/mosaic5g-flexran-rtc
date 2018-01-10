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

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <boost/thread/barrier.hpp>

#include "rt_task.h"
#include "rib.h"
#include "requests_manager.h"

namespace flexran {

  namespace app {

    class component : public core::rt::rt_task {
    public:
  
    component(rib::Rib& rib, const core::requests_manager& rm)
      : rt_task(Policy::DEADLINE, 8 * 100 * 1000, 8 * 100 * 1000, 1000 * 1000), rib_(rib), req_manager_(rm) {}

      //! the barrier is for synchronization purposes. The task manager uses
      //  this to inform apps when they can run.
      void set_app_sync_barrier(std::shared_ptr<boost::barrier> barrier) { app_sync_barrier_ = barrier; }

      virtual void run_app() = 0;

    protected:
      const rib::Rib& rib_;
      const core::requests_manager& req_manager_;
      std::shared_ptr<boost::barrier> app_sync_barrier_;
  
    private:
      
      void run() { run_app(); }
      
    };

  }

}

#endif

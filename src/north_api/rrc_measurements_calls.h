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

#ifndef FLEXIBLE_SCHED_CALLS_H_
#define FLEXIBLE_SCHED_CALLS_H_

#include <pistache/http.h>

#include "app_calls.h"
#include "rrc_measurements.h"

namespace flexran {

  namespace north_api {

    class flexible_rrc_calls : public app_calls {

    public:

      flexible_rrc_calls(std::shared_ptr<flexran::app::rrc::rrc_measurements> flex_sched)
	: rrc_trigger(flex_sched)
      { }
      
      void register_calls(Net::Rest::Router& router);

      void change_rrc(const Net::Rest::Request& request, Net::Http::ResponseWriter response);

    private:

      std::shared_ptr<flexran::app::rrc::rrc_measurements> rrc_trigger;

    };
  }
}


#endif /* FLEXIBLE_SCHED_CALLS_H_ */
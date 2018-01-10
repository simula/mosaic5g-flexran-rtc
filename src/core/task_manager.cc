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

#include <vector>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <boost/thread/barrier.hpp>

#include "task_manager.h"
#include "flexran_log.h"

extern std::atomic_bool g_exit_controller;

#ifdef PROFILE
#include <chrono>
#include <fstream>
#include <sstream>

extern std::atomic_bool g_startprof;
#endif

flexran::core::task_manager::task_manager(flexran::rib::rib_updater& r_updater)
  : rt_task(Policy::FIFO, 80), r_updater_(r_updater) {
  struct itimerspec its;
  
  sfd = timerfd_create(CLOCK_MONOTONIC, 0);
  
  /* Start the timer */
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1000 * 1000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  
  if (timerfd_settime(sfd, TFD_TIMER_ABSTIME, &its, NULL) == -1) {
    LOG4CXX_ERROR(flog::core, "Failed to set timer for task manager");
  }
}

void flexran::core::task_manager::run() {
  manage_rt_tasks();
}

void flexran::core::task_manager::manage_rt_tasks() {
  // create a barrier for all apps plus the task manager
  std::shared_ptr<boost::barrier> app_sync_barrier = std::make_shared<boost::barrier>(apps_.size() + 1);
  std::vector<std::thread> running_apps;
  for (auto& app: apps_) {
    app->set_app_sync_barrier(app_sync_barrier);
    running_apps.push_back(std::thread(&flexran::app::component::execute_task, app));
  }

#ifdef PROFILE
  std::chrono::steady_clock::time_point loop_start, app_start;
  std::chrono::duration<float, std::micro> rib_dur, app_dur, loop_dur, inter_dur;

  std::stringstream ss;
  int rounds = 30000;
#endif

  while (!g_exit_controller) {
#ifdef PROFILE
    inter_dur = std::chrono::steady_clock::now() - loop_start;
    loop_start = std::chrono::steady_clock::now();
#endif

    // First run the RIB updater
    r_updater_.run();

#ifdef PROFILE
    app_start = std::chrono::steady_clock::now();
    rib_dur = app_start - loop_start;
#endif

    // Then spawn any registered application and wait for them to finish.
    app_sync_barrier->wait();
    app_sync_barrier->wait();

#ifdef PROFILE
    app_dur = std::chrono::steady_clock::now() - app_start;
    loop_dur = std::chrono::steady_clock::now() - loop_start;
    if (g_startprof && rounds > 0) {
      ss << inter_dur.count() << "\t" << loop_dur.count() << "\t"
         << rib_dur.count() << "\t" << app_dur.count() << "\n";
      rounds--;
      if (rounds == 0)
        LOG4CXX_WARN(flog::core, "DONE");
    }
#endif
    wait_for_cycle();
  }

#ifdef PROFILE
  std::ofstream fstat;
  std::string fn = "new." + std::to_string(apps_.size()) + ".dat";
  LOG4CXX_WARN(flog::core, "writing to file " << fn);
  fstat.open(fn);
  fstat << ss.rdbuf();
  fstat.close();
#endif

  // release all apps
  for (auto& app: apps_)
    app->inform_exit();
  app_sync_barrier->wait();
  for (auto& thread: running_apps)
    thread.join();
}

// Warning: Not thread safe for the moment
void flexran::core::task_manager::register_app(const std::shared_ptr<flexran::app::component>& app) {
  apps_.emplace_back(app);
}

void flexran::core::task_manager::wait_for_cycle() {
  uint64_t exp;
  ssize_t res;

  if (sfd > 0) {
    res = read(sfd, &exp, sizeof(exp));

    if ((res < 0) || (res != sizeof(exp))) {
      LOG4CXX_ERROR(flog::core, "Failed in task manager timer wait");
    }
  }
}

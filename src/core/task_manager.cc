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

/*! \file    task_manager.cc
 *  \brief   handles the tasks within the controller
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
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
#include <iomanip>
#ifdef LOWLATENCY
#include <linux/types.h>
#include <sched.h>
#include <cerrno>
#include <cstring>
#endif

extern std::atomic_bool g_doprof;
extern std::chrono::time_point<std::chrono::steady_clock> start;
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

  std::chrono::steady_clock::time_point loop_start;
  std::chrono::duration<float, std::micro> loop_dur;
#ifdef PROFILE
  std::chrono::steady_clock::time_point app_start;
  std::chrono::duration<float, std::micro> rib_dur, app_dur, inter_dur;

  std::unique_ptr<std::stringstream> ss(nullptr);
  int rounds = 10000;
  unsigned int processed;
#endif

  while (!g_exit_controller) {
#ifdef PROFILE
    inter_dur = std::chrono::steady_clock::now() - loop_start;
#endif
    loop_start = std::chrono::steady_clock::now();

    // First run the RIB updater
#ifdef PROFILE
    processed =
#endif
    r_updater_.run();

#ifdef PROFILE
    app_start = std::chrono::steady_clock::now();
    rib_dur = app_start - loop_start;
#endif

    // Then spawn any registered application and wait for them to finish.
    app_sync_barrier->wait();
    app_sync_barrier->wait();

    loop_dur = std::chrono::steady_clock::now() - loop_start;
    if (loop_dur.count() > 990)
      LOG4CXX_WARN(flog::app, "task_manager: loop duration was "
          << loop_dur.count() << " us");
#ifdef PROFILE
    app_dur = std::chrono::steady_clock::now() - app_start;
    loop_dur = std::chrono::steady_clock::now() - loop_start;
    if (g_doprof) {
      if (!ss) {
        LOG4CXX_WARN(flog::core, "start profiling task_manager");
        ss.reset(new std::stringstream);
      }
      *ss << inter_dur.count() << "\t" << loop_dur.count() << "\t"
          << rib_dur.count() << "\t" << processed << "\t" << app_dur.count() << "\n";
      rounds--;
      if (rounds == 0) {
        g_doprof = false;
        rounds = 10000;
        std::thread t(flexran::core::task_manager::profiler_wb_thread, std::move(ss), apps_.size());
        t.detach();
        LOG4CXX_WARN(flog::core, "profiling done");
        r_updater_.print_prof_results(std::chrono::steady_clock::now() - start);
      }
    }
#endif
    wait_for_cycle();
  }

  // release all apps
  for (auto& app: apps_)
    app->inform_exit();
  app_sync_barrier->wait();
  for (auto& thread: running_apps)
    thread.join();
}

void flexran::core::task_manager::register_app(const std::shared_ptr<flexran::app::component>& app)
{
  apps_.emplace_back(app);
}


#ifdef PROFILE
void flexran::core::task_manager::profiler_wb_thread(
    std::unique_ptr<std::stringstream> ss, size_t num_apps)
{
#ifdef LOWLATENCY
  /* set back priority to standard */
  struct sched_param sp;
  sp.sched_priority = 0;
  int ret = sched_setscheduler(0, SCHED_OTHER, &sp);
  if (ret != 0) {
    LOG4CXX_ERROR(flog::core, "error for sched_setscheduler(): "
        << std::strerror(errno));
  }
#endif

  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream h;
  h << std::put_time(std::localtime(&now), "%H.%M.%S");
  std::string fn = "tm.profile." + std::to_string(num_apps) + "." + h.str() + ".dat";
  std::ofstream fstat;
  LOG4CXX_WARN(flog::core, "writing to file " << fn);
  fstat.open(fn);
  if (!fstat.is_open()) {
    LOG4CXX_ERROR(flog::core, "can not open file for writing profiling info");
    return;
  }
  fstat << "inter loop\tinner loop dur\trib dur\tprocessed\tapp dur\n";
  fstat << ss->rdbuf();
  fstat.close();
}
#endif

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

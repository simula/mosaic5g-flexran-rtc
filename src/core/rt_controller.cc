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

/*! \file    rt_controller.cc
 *  \brief   rt_controller main file
 *  \authors Xenofon Foukas, Navid Nikaein, Shahab SHARIAT BAGHERI, Robert
 *           Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, navid.nikaein@eurecom.fr,
 *           shahab.shariat@eurecom.fr, robert.schmidt@eurecom.fr
 */

#include <thread>
#include <iostream>

#include <pthread.h>
#include <sched.h>
#include <linux/sched.h>

#include <boost/program_options.hpp>

#include "rt_wrapper.h"

#include "async_xface.h"
#include "flexran.pb.h"
#include "rib_updater.h"
#include "rib.h"
#include "task_manager.h"
#include "subscription.h"
#include "stats_manager.h"
#include "rrm_management.h"
//#include "remote_scheduler.h"
//#include "remote_scheduler_delegation.h"
//#include "remote_scheduler_eicic.h"
//#include "flexible_scheduler.h"
#include "rrc_triggering.h"
//#include "delegation_manager.h"
#include "requests_manager.h"
#include "rib_management.h"
#include "recorder.h"

//#ifdef NEO4J_SUPPORT
//#include "neo4j_client.h"
//#endif

#ifdef ELASTIC_SEARCH_SUPPORT
#include "elastic_search.h"
#endif

// Fort RESTful northbound API
#ifdef REST_NORTHBOUND

#include "call_manager.h"
#include "rrm_calls.h"
#include "rrc_triggering_calls.h"
#include "stats_manager_calls.h"
#include "recorder_calls.h"
#ifdef ELASTIC_SEARCH_SUPPORT
#include "elastic_calls.h"
#endif

#endif

#ifdef REST_NORTHBOUND
#include <pistache/endpoint.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "flexran_log.h"

std::atomic_bool g_exit_controller{false};

#ifdef PROFILE
std::atomic_bool g_doprof{false};
std::chrono::time_point<std::chrono::steady_clock> start;
#endif

namespace po = boost::program_options;

int main(int argc, char* argv[]) {

  int cport = 2210;
#ifdef REST_NORTHBOUND
  int north_port = 9999;
#endif
  
  bool debug = false;

  bool expl_log_config = false;

  sigset_t sigmask;
  int rc, sig;

  // Find the root directory
  std::string path = "";

  if(const char* env_p = std::getenv("FLEXRAN_RTC_HOME")) {
    path = env_p;
  } else {
    path = "../";
  }

  try {
    po::options_description desc("Help");
    desc.add_options()
      ("config,c", po::value<std::string>(), "Path to logger configuration file. "
       "Without it, FLEXRAN_RTC_HOME or ../ is tried")
      ("debug,d", "Enables debugging messages to be displayed and logged")
      ("help,h", "Prints this help message")
      ("nport,n", po::value<int>()->default_value(9999),
       "Port for northbound API calls")
      ("port,p", po::value<int>()->default_value(2210),
       "Port for incoming agent connections");
    
    po::variables_map opts;
    po::store(po::parse_command_line(argc, argv, desc), opts);

    if ( opts.count("help")  ) { 
      std::cout << "FlexRAN real-time controller" << std::endl 
                << desc << std::endl;
      return 0; 
    } 

    if (opts.count("debug")) {
      std::cout << "Debugging enabled" << std::endl;
      debug = true;
    }

    if (opts.count("config")) {
      expl_log_config = true;
      path = opts["config"].as<std::string>();
    }
    
    try {
      po::notify(opts);
    } catch (po::error& e) {
      std::cerr << "Error: Unrecognized parameter\n";
      return 1;
    }
    
    cport = opts["port"].as<int>(); 
#ifdef REST_NORTHBOUND
    north_port = opts["nport"].as<int>();
#endif
    
  } catch(std::exception& e) {
    std::cerr << "Error: Unrecognized parameter\n";
    return 2;
  } 

  if (expl_log_config) {
    // Initialize the logger with custom config file from parameters in path
    flexran_log::PropertyConfigurator::configure(path);
  } else if (!debug) {
    // Initialize the logger with default properties
    flexran_log::PropertyConfigurator::configure(path + "/log_config/basic_log");
  } else {
    // Initialize the logger with debugging properties
    flexran_log::PropertyConfigurator::configure(path + "/log_config/debug_log");
  }

#ifdef PROFILE
  LOG4CXX_WARN(flog::core, "Compiled with profiling support (send USR2 to controller)");
#endif
    
  LOG4CXX_INFO(flog::core, "Listening on port " << cport << " for incoming agent connections");
  flexran::network::async_xface net_xface(cport);
  
  // Create the rib
  flexran::rib::Rib rib;

  // Create the requests manager
  flexran::core::requests_manager rm(rib, net_xface);

  // Create the event subsystem
  flexran::event::subscription ev;

  // Create the rib update manager
  flexran::rib::rib_updater r_updater(rib, net_xface, rm, ev);

  // Create the task manager
  flexran::core::task_manager tm(r_updater, ev);

  // Register any applications that we might want to execute in the controller
  auto stats_app = std::make_shared<flexran::app::stats::stats_manager>(rib, rm, ev);
  auto rrm_management = std::make_shared<flexran::app::management::rrm_management>(rib, rm, ev);
  auto rrc_trigger = std::make_shared<flexran::app::rrc::rrc_triggering>(rib, rm, ev);
  auto rib_management = std::make_shared<flexran::app::management::rib_management>(rib, rm, ev);
  auto recorder = std::make_shared<flexran::app::log::recorder>(rib, rm, ev);

  /* More examples of developed applications are available in the commented section.
     WARNING: Some of them might still contain bugs or might be from previous versions of the controller. */
  //auto remote_sched = std::make_shared<flexran::app::scheduler::remote_scheduler>(rib, rm, ev);
  //auto remote_sched_eicic = std::make_shared<flexran::app::scheduler::remote_scheduler_eicic>(rib, rm, ev);
  //auto remote_sched = std::make_shared<flexran::app::scheduler::remote_scheduler_delegation>(rib, rm, ev);
  //auto delegation_manager = std::make_shared<flexran::app::management::delegation_manager>(rib, rm, ev);
//#ifdef NEO4J_SUPPORT
  //auto n4j_client = std::make_shared<flexran::app::management::neo4j_client>(rib, rm, ev);
//#endif

#ifdef ELASTIC_SEARCH_SUPPORT
  auto elastic = std::make_shared<flexran::app::log::elastic_search>(rib, rm, ev);
#endif

  // the following threads will not handle any signal
  sigfillset(&sigmask);
  rc = pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
  if (rc) {
    LOG4CXX_FATAL(flog::core, "Can not set pthread_sigmask due to error " << rc
        << ". Exiting");
    exit(rc);
  }

  // Start the task manager thread
  std::thread task_manager_thread(&flexran::core::task_manager::execute_task, &tm);

  // Start the network thread
  std::thread networkThread(&flexran::network::async_xface::execute_task, &net_xface);

#ifdef REST_NORTHBOUND
  // Set the port and the IP to listen for REST calls and initialize the call manager
  Pistache::Port port(north_port);
  Pistache::Address addr(Pistache::Ipv4::any(), port);
  flexran::north_api::manager::call_manager north_api(addr);

  flexran::north_api::rrm_calls rrm_calls(rrm_management);
  north_api.register_calls(rrm_calls);
  flexran::north_api::stats_manager_calls stats_calls(stats_app);
  north_api.register_calls(stats_calls);
  flexran::north_api::recorder_calls recorder_calls(recorder);
  north_api.register_calls(recorder_calls);
  flexran::north_api::rrc_triggering_calls rrc_calls(rrc_trigger);
  north_api.register_calls(rrc_calls);
#ifdef ELASTIC_SEARCH_SUPPORT
  flexran::north_api::elastic_calls elastic_calls(elastic);
  north_api.register_calls(elastic_calls);
#endif

  // Start the call manager threaded. Once task_manager_thread and
  // networkThread return, north_api will be shut down too
  north_api.init(1);
  north_api.start();
  LOG4CXX_INFO(flog::core, "Listening on port " << north_port << " for incoming"
      << " REST connections");
#endif

  // handle SIGINT and SIGUSR1 as end signals
  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGINT);
  sigaddset(&sigmask, SIGUSR1);
#ifdef PROFILE
  sigaddset(&sigmask, SIGUSR2);
#endif
  sigaddset(&sigmask, SIGTERM);
  pthread_sigmask(SIG_SETMASK, &sigmask, NULL);
  while (!g_exit_controller) {
    rc = sigwait(&sigmask, &sig);
    if (rc) {
      LOG4CXX_FATAL(flog::core, "sigwait() error code " << rc << ". Exiting");
      exit(rc);
    }
    if (sig == SIGINT || sig == SIGUSR1 || sig == SIGTERM) {
      g_exit_controller = true;
    }
#ifdef PROFILE
    if (sig == SIGUSR2) {
      if (!g_doprof) {
        LOG4CXX_INFO(flog::core, "start profiling");
        g_doprof = true;
        start = std::chrono::steady_clock::now();
      } else
        LOG4CXX_ERROR(flog::core, "profiler already running");
    }
#endif
  }

  if (task_manager_thread.joinable())
    task_manager_thread.join();
  
  net_xface.end();
  if (networkThread.joinable())
    networkThread.join();

#ifdef REST_NORTHBOUND
  north_api.shutdown();
#endif

  LOG4CXX_INFO(flog::core, "Exiting FlexRAN RTController, bye.");

  return 0;
}

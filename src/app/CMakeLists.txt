set(APP_SRC
    aqm_management.cc
    stats_manager.cc
    rrc_triggering.cc
    rib_management.cc
    recorder.cc
    rrm_management.cc
    band_check.cc
#   flexible_scheduler.cc
#   remote_scheduler.cc
#   remote_scheduler_helper.cc
#   remote_scheduler_primitives.cc
#   enb_scheduling_info.cc
#   ue_scheduling_info.cc
#   remote_scheduler_delegation.cc
#   delegation_manager.cc
#   remote_scheduler_eicic.cc
#   enb_scheduler_policy.cc
)

#if(NEO4J_SUPPORT)
#  set(NEO4J_INCLUDE "/usr/include/neo4j-client.h")
#  if(EXISTS ${NEO4J_INCLUDE})
#    message("--${NEO4J_INCLUDE} FOUND")
#  else()
#    message(FATAL_ERROR " --${NEO4J_INCLUDE} NOT FOUND. There is no find_package(),
#    and it is not in /usr/include. If you are sure it is installed, consider
#    changing the cmake file (search for 'NEO4J_SUPPORT')")
#  endif()
#  set(APP_SRC ${APP_SRC} neo4j_client.cc)
#endif(NEO4J_SUPPORT)

if(ELASTIC_SEARCH_SUPPORT)
  find_package(CURL REQUIRED)
  #include_directories(${CURL_INCLUDE_DIRS})
  #link_directories(${CURL_LIBRARY_DIRS})
  set(APP_SRC ${APP_SRC} elastic_search.cc)
endif()

add_library(RTC_APP_LIB ${APP_SRC})

target_include_directories(RTC_APP_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RTC_APP_LIB
  RTC_NETWORK_LIB
  RTC_RIB_LIB
)
if(ELASTIC_SEARCH_SUPPORT)
  target_link_libraries(RTC_APP_LIB ${CURL_LIBRARIES})
endif()

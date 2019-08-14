find_package(Pistache REQUIRED)
set(PISTACHE_INCLUDE_DIRS /usr/local/include/pistache)
set(PISTACHE_LIB /usr/local/lib/libpistache.a)
include_directories(${PISTACHE_INCLUDE_DIRS})

add_library(RTC_NORTH_API_LIB
    call_manager.cc
    rrm_calls.cc
    stats_manager_calls.cc
    rrc_triggering_calls.cc
    recorder_calls.cc
)
if(ELASTIC_SEARCH_SUPPORT)
  target_sources(RTC_NORTH_API_LIB PRIVATE elastic_calls.cc)
endif()

target_include_directories(RTC_NORTH_API_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RTC_NORTH_API_LIB
  PRIVATE RTC_APP_LIB RTC_CORE_LIB ${PISTACHE_LIB}
)

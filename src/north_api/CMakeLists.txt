find_package(Pistache REQUIRED)
set(PISTACHE_INCLUDE_DIRS /usr/local/include/pistache)
set(PISTACHE_LIB /usr/local/lib/libpistache.a)
include_directories(${PISTACHE_INCLUDE_DIRS})

set(NORTH_API_SRC
    aqm_calls.cc
    call_manager.cc
    elastic_calls.cc
    rrm_calls.cc
    stats_manager_calls.cc
    rrc_triggering_calls.cc
    recorder_calls.cc
)

add_library(RTC_NORTH_API_LIB ${NORTH_API_SRC})

target_include_directories(RTC_NORTH_API_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RTC_NORTH_API_LIB
  RTC_APP_LIB
  ${PISTACHE_LIB}
)

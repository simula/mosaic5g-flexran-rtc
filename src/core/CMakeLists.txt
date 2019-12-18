add_library(RTC_CORE_LIB
  rt_wrapper.cc
  task_manager.cc
  rt_task.cc
  requests_manager.cc
)	

configure_file("rtc_version.h.in" "rtc_version.h")

target_include_directories(RTC_CORE_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(RTC_CORE_LIB
  PRIVATE FLPT_MSG_LIB RTC_APP_LIB RTC_NETWORK_LIB RTC_RIB_LIB
    Boost::system
    Boost::program_options
  PUBLIC RTC_EVENT_LIB ${Log4CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(rt_controller rt_controller.cc)
target_link_libraries(rt_controller
  PRIVATE RTC_APP_LIB RTC_CORE_LIB RTC_NETWORK_LIB)
if(REST_NORTHBOUND)
  target_link_libraries(rt_controller PRIVATE RTC_NORTH_API_LIB)
endif()
add_custom_command(TARGET rt_controller POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy rt_controller ${PROJECT_BINARY_DIR}/.
)

add_executable(parse-bd parse-bd.cc)
target_link_libraries(parse-bd PRIVATE RTC_APP_LIB RTC_CORE_LIB)
add_custom_command(TARGET parse-bd POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy parse-bd ${PROJECT_BINARY_DIR}/.
)

set(RT_CONTROLLER_CORE_SRC
    rt_wrapper.cc
    task_manager.cc
    rt_task.cc
    requests_manager.cc
)	

add_library(RTC_CORE_LIB ${RT_CONTROLLER_CORE_SRC})	

target_include_directories(RTC_CORE_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RTC_CORE_LIB PUBLIC
  RTC_NETWORK_LIB
  RTC_EVENT_LIB
  RTC_RIB_LIB
  RTC_APP_LIB
  FLPT_MSG_LIB
)
if(REST_NORTHBOUND)
  target_link_libraries(RTC_CORE_LIB PUBLIC RTC_NORTH_API_LIB)
endif()

add_executable(rt_controller rt_controller.cc)
target_link_libraries(rt_controller
  RTC_CORE_LIB
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Log4CXX_LIBRARIES}
)
add_custom_command(TARGET rt_controller POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy rt_controller ${PROJECT_BINARY_DIR}/.
)

add_executable(parse-bd ../../tools/parse-bd.cc)
target_link_libraries(parse-bd
  RTC_APP_LIB
  RTC_CORE_LIB
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Log4CXX_LIBRARIES}
)
add_custom_command(TARGET parse-bd POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy parse-bd ${PROJECT_BINARY_DIR}/.
)

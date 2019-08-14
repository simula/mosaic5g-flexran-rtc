set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch2::Catch IMPORTED INTERFACE)
#set_property(Catch2::Catch PROPERTY INTERFACE_INCLUDE_DIRECTORIES
#"${CATCH_INCLUDE_DIR}")

set(RT_CONTROLLER_TEST_SRC
    agent_capabilities.cc
    app_recorder.cc
    app_rrm_management.cc
    enb_rib_info.cc
    rib.cc
    test.cc
)

add_executable(rtc_test ${RT_CONTROLLER_TEST_SRC})
target_link_libraries(rtc_test
  RTC_APP_LIB
  RTC_CORE_LIB
  ${Log4CXX_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  Catch2::Catch
)

add_test(NAME rtc_test_name COMMAND rtc_test)
add_custom_command(TARGET rtc_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy rtc_test ${PROJECT_BINARY_DIR}/.
)

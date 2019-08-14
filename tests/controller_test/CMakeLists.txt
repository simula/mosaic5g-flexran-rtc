set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch2::Catch IMPORTED INTERFACE)

add_executable(rtc_test
  agent_capabilities.cc
  app_recorder.cc
  app_rrm_management.cc
  enb_rib_info.cc
  rib.cc
  test.cc
)
target_link_libraries(rtc_test
  RTC_APP_LIB
  RTC_CORE_LIB
  Catch2::Catch
)

add_test(NAME rtc_test_name COMMAND rtc_test)
add_custom_command(TARGET rtc_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy rtc_test ${PROJECT_BINARY_DIR}/.
)

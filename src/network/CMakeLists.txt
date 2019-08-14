add_library(RTC_NETWORK_LIB
  async_xface.cc
  connection_manager.cc
  agent_session.cc
  protocol_message.cc
  tagged_message.cc
)

target_include_directories(RTC_NETWORK_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RTC_NETWORK_LIB PRIVATE FLPT_MSG_LIB RTC_CORE_LIB)

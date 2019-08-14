add_library(RTC_RIB_LIB
  agent_info.cc
  cell_mac_rib_info.cc
  enb_rib_info.cc
  rib.cc
  rib_common.cc
  rib_updater.cc
  ue_mac_rib_info.cc
)

target_include_directories(RTC_RIB_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RTC_RIB_LIB
  PRIVATE RTC_CORE_LIB RTC_NETWORK_LIB
  PUBLIC FLPT_MSG_LIB RTC_EVENT_LIB
)

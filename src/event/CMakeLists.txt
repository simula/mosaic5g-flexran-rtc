add_library(RTC_EVENT_LIB subscription.cc)
target_include_directories(RTC_EVENT_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RTC_EVENT_LIB PUBLIC RTC_RIB_LIB)

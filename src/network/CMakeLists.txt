set(NETWORK_SRC
    async_xface.cc
    connection_manager.cc
    agent_session.cc
    protocol_message.cc
    tagged_message.cc
)

add_library(RTC_NETWORK_LIB ${NETWORK_SRC})
target_link_libraries(RTC_NETWORK_LIB
    RTC_CORE_LIB
    FLPT_MSG_LIB
)
target_include_directories(RTC_NETWORK_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

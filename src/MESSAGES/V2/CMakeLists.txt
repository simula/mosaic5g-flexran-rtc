include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER
  config_common.proto
  config_messages.proto
  control_delegation.proto
  controller_commands.proto
  flexran.proto
  header.proto
  mac_primitives.proto
  stats_common.proto
  stats_messages.proto
  time_common.proto
)

add_library(FLPT_MSG_LIB ${PROTO_SRC} ${PROTO_HEADER})
target_include_directories(FLPT_MSG_LIB PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(FLPT_MSG_LIB PUBLIC ${PROTOBUF_LIBRARY})
# disable unused parameter ‘deterministic’ [-Wunused-parameter] warnings
target_compile_options(FLPT_MSG_LIB PRIVATE -Wno-unused-parameter)

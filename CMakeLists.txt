cmake_minimum_required (VERSION 3.5)

project (rt_controller VERSION 2.2.1 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 14)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# force compile options on all targets
set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

option(LOWLATENCY "Enable soft-real time scheduling (for Linux kernel 3.14 and above)" ON)
if(LOWLATENCY)
  add_definitions(-DLOWLATENCY)
endif()

# Set constants for abstract simulation mode
#if(ABSTRACT_SIMULATION)
#add_definitions(-DABSTRACT_SIMULATION)
#endif()

# Enable RESTful northbound API
option(REST_NORTHBOUND "Enable RESTful northbound API for the management of the controller" ON)
if(REST_NORTHBOUND)
  add_definitions(-DREST_NORTHBOUND)
endif()

# Enable Neo4j app
#option(NEO4J_SUPPORT "Enable graphing application" OFF)
#if(NEO4J_SUPPORT)
#  add_definitions(-DNEO4J_SUPPORT)
#endif()

# Enable Elastic Search app
option(ELASTIC_SEARCH_SUPPORT "Enable Elastic Search application" ON)
if(ELASTIC_SEARCH_SUPPORT)
  add_definitions(-DELASTIC_SEARCH_SUPPORT)
endif()

# Add Profiling support
option(PROFILE "Enable profiling of task_manager" OFF)
if(PROFILE)
  add_definitions(-DPROFILE)
endif()

set(Log4CXX_DIR "${CMAKE_SOURCE_DIR}/cmake_modules")
find_package(Log4CXX REQUIRED)

find_package(Boost 1.54.0 REQUIRED COMPONENTS system program_options)

find_package(Threads)

add_subdirectory(src)

option(ENABLE_TESTS "Enable tests" OFF)
if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests/controller_test)
endif()

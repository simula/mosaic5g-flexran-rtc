cmake_minimum_required (VERSION 3.5)

project (rt_controller CXX)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g")

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


#Find threads library
find_package (Threads)

#Find log4cxx library
set(Log4CXX_DIR "${CMAKE_SOURCE_DIR}/cmake_modules")
find_package(Log4CXX REQUIRED)
include_directories(${Log4CXX_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.54.0 REQUIRED COMPONENTS system program_options)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


add_subdirectory(src)

option(ENABLE_TESTS "Enable tests" OFF)
if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests/controller_test)
endif()

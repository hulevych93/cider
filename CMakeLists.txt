# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22)

project(cider)

set(CIDER_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CIDER_BINARY_DIR ${PROJECT_BINARY_DIR})

include (cmake/main.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DRESULTS_VERSION_2)

include_directories(${CMAKE_SOURCE_DIR})

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
  message(FATAL_ERROR "cider only works fine in DEBUG build.")
endif()

# Try to find Python 3.11 first
  find_program(PYTHON3_11_EXECUTABLE NAMES python3.11)

if(PYTHON3_11_EXECUTABLE)
  message(STATUS "Using Python 3.11: ${PYTHON3_11_EXECUTABLE}")
  set(Python3_EXECUTABLE ${PYTHON3_11_EXECUTABLE} CACHE FILEPATH "Use Python 3.11 if available")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

message("Python ${Python3_EXECUTABLE}")

OPTION(ENABLE_TESTS "Enable cider unit-tests" OFF)

enable_testing()

add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/swig)
add_subdirectory(thirdparty/lua)
add_subdirectory(thirdparty/cppast)
add_subdirectory(thirdparty/cxxopts)
add_subdirectory(thirdparty/nlohmann)
add_subdirectory(thirdparty/pybind)
add_subdirectory(thirdparty/tpl)
add_subdirectory(thirdparty/tinylog)

add_subdirectory(thirdparty/mathplotlib)
add_subdirectory(mathplot-log)

add_subdirectory(synthesis)
add_subdirectory(coverage)
add_subdirectory(serialization)
add_subdirectory(metaheuristics)
add_subdirectory(agent-model)
add_subdirectory(agent-q-learning)
add_subdirectory(agent-sarsa-learning)
add_subdirectory(monte-carlo-tree-search)
add_subdirectory(dynamic-slicer)
add_subdirectory(greedy-r)
add_subdirectory(pipelines)
add_subdirectory(math)

add_subdirectory(tools/cider)

add_subdirectory(recorder)
add_subdirectory(scripting)

if(ENABLE_TESTS)
  add_subdirectory(thirdparty/gtest)

  add_subdirectory(tests)
  add_subdirectory(recorder/tests)
  add_subdirectory(serialization/tests)
  add_subdirectory(agent-model/tests)
endif()

include(cmake/3rd/coverage.cmake)
append_coverage_compiler_flags()

add_subdirectory(experiments)

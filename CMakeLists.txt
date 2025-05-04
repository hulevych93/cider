# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22)

project(cider)

set(CIDER_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CIDER_BINARY_DIR ${PROJECT_BINARY_DIR})

include (cmake/main.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

OPTION(ENABLE_COVERAGE "Without this coverage won't be meassured" ON)
OPTION(CIDER_ENABLE_COVERAGE "Coverage target for cider tests" OFF)
OPTION(ENABLE_MATHPLOT_LOG "Enable online results plotting" OFF)

enable_testing()

add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/swig)
add_subdirectory(thirdparty/lua)
add_subdirectory(thirdparty/cppast)
add_subdirectory(thirdparty/cxxopts)
add_subdirectory(thirdparty/nlohmann)
add_subdirectory(thirdparty/tpl)

if(ENABLE_MATHPLOT_LOG)
    # Try to find Python 3.11 first
    find_program(PYTHON3_11_EXECUTABLE NAMES python3.11)

    if(PYTHON3_11_EXECUTABLE)
        message(STATUS "Using Python 3.11: ${PYTHON3_11_EXECUTABLE}")
        set(Python3_EXECUTABLE ${PYTHON3_11_EXECUTABLE} CACHE FILEPATH "Use Python 3.11 if available")
    endif()

    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

    message("Python ${Python3_EXECUTABLE}")

    add_subdirectory(thirdparty/mathplotlib)
endif()

add_subdirectory(coverage)
add_subdirectory(metaheuristics)
add_subdirectory(q-learning)
add_subdirectory(tools/cider)
add_subdirectory(pipelines)
add_subdirectory(tools/coverage/table_printer)

if(ENABLE_COVERAGE)
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
      message(FATAL_ERROR "CIDER_ENABLE_COVERAGE only works fine in DEBUG build.")
    endif()
  include(cmake/3rd/coverage.cmake)
  append_coverage_compiler_flags()
endif()

add_subdirectory(recorder)
add_subdirectory(scripting)

if(ENABLE_TESTS)
  add_subdirectory(thirdparty/gtest)

  add_subdirectory(tests)
  add_subdirectory(recorder/tests)
endif()

if(CIDER_ENABLE_COVERAGE)
  setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE ctest
        DEPENDENCIES recorder_test tests
        EXCLUDE "tools/*" "tests/models/*" "${BUILD_FILES_DIR}/*" "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*")
endif()

add_subdirectory(experiments/hjson)
add_subdirectory(experiments/pugixml)
add_subdirectory(experiments/BitmapPlusPlus)

add_custom_target(run_all_HS
    DEPENDS
        bitmap++_HS_run
        hjson_HS_run
        pugixml_HS_run
)

add_custom_target(run_all_CS
    DEPENDS
        bitmap++_CS_run
        hjson_CS_run
        pugixml_CS_run
)

add_custom_target(run_all_QL
    DEPENDS
        bitmap++_QL_run
        hjson_QL_run
        pugixml_QL_run
)

add_custom_target(run_all
    DEPENDS
        run_all_HS
        run_all_CS
        run_all_QL
)

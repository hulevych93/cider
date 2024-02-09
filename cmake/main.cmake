# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(CMAKE_CXX_STANDARD 14)

# This tells CMake that we want "-std=c++XX" and not "-std=gnu++XX"
set(CMAKE_CXX_EXTENSIONS FALSE)
# This disables the fallback to an earlier standard if the specified one is not supported by the given compiler.
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if(WIN32 OR APPLE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/bin)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endforeach()

if(NOT DEFINED CIDER_ROOT_DIR)
    message(FATAL_ERROR "Please, define CIDER_ROOT_DIR cmake variable.")
endif()

# The `BUILD_FILES_ROOT` is the directory where all the
# project 3rd parties are supposed to be build.
if(DEFINED ENV{BUILD_FILES_ROOT})
    file(TO_CMAKE_PATH $ENV{BUILD_FILES_ROOT} BUILD_FILES_DIR)
else()
     message("Please, define BUILD_FILES_ROOT env variable.")
     set(BUILD_FILES_DIR ${CMAKE_BINARY_DIR}/external)
endif()

if((NOT "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG") AND (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE"))
    message(WARNING "CMAKE_BUILD_TYPE is not set correctly, setting it to DEBUG")
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

set(CIDER_CMAKE_DIR ${CIDER_ROOT_DIR}/cmake)

include(${CIDER_CMAKE_DIR}/common.cmake)

# This is the first target which we execute before building our code.
# Note: third-parties are built before this target.
if(NOT DEFINED CIDER_THIRDPARTY_TARGET_NAME)
    cider_export_var(CIDER_THIRDPARTY_TARGET_NAME build_thirdparty)
    add_custom_target(${CIDER_THIRDPARTY_TARGET_NAME})
endif()

include(${CIDER_CMAKE_DIR}/cider.cmake)
include(${CIDER_CMAKE_DIR}/vars.cmake)
include(${CIDER_CMAKE_DIR}/sources.cmake)
include(${CIDER_CMAKE_DIR}/thirdparty.cmake)
include(${CIDER_CMAKE_DIR}/clang-format.cmake)

if(UNIX)
    set(PLATFORM_LIBRARIES dl pthread)
endif()


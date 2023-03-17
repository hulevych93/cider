cmake_minimum_required(VERSION 3.22)

project(gunit)

set(GUNIT_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(GUNIT_BINARY_DIR ${PROJECT_BINARY_DIR})

include (cmake/main.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

enable_testing()

add_subdirectory(thirdparty/gtest)
add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/swig)
add_subdirectory(thirdparty/lua)
add_subdirectory(thirdparty/boost) #TODO get rid off boost
add_subdirectory(thirdparty/cppast)
add_subdirectory(thirdparty/cxxopts)

if(GUNIT_ENABLE_COVERAGE)
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
      message(FATAL_ERROR "GUNIT_ENABLE_COVERAGE only works fine in DEBUG build.")
    endif()
  include(cmake/3rd/coverage.cmake)
  append_coverage_compiler_flags()
endif()

add_subdirectory(recorder)
add_subdirectory(scripting)
add_subdirectory(tools/gunit)
add_subdirectory(tests)

if(GUNIT_ENABLE_COVERAGE)
  setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE ctest
        DEPENDENCIES recorder_test tests
        EXCLUDE "tools/*" "tests/models/*" "${BUILD_FILES_DIR}/*" "${GUNIT_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*")
endif()

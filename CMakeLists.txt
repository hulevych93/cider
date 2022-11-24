cmake_minimum_required(VERSION 3.22)

project(gunit)

set(GUNIT_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(GUNIT_BINARY_DIR ${PROJECT_BINARY_DIR})

include (cmake/main.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(thirdparty/gtest)
add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/swig)
add_subdirectory(thirdparty/lua)

add_subdirectory(models)
add_subdirectory(recorder)
add_subdirectory(scripting)

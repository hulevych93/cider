# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_driver)

add_executable(hjson_driver coverage_driver.cpp)

target_link_libraries(hjson_driver PUBLIC
  coverage
  harmony
  hjson_tests
)

if(ENABLE_COVERAGE_DRIVER)
  add_custom_target(hjson_coverage_driver_run
      COMMAND hjson_driver
      "${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests"
      "${EXPERIMENT_HJSON_BUILD_DIR}/src"
      "${CMAKE_BINARY_DIR}/experiments/hjson/tests/player"
      "${CMAKE_BINARY_DIR}/bin/hjson_player"
      "${CMAKE_BINARY_DIR}"

      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests"
      DEPENDS hjson_player
      VERBATIM # Protect arguments to commands
  )
endif()

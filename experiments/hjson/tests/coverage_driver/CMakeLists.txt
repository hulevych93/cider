# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(coverage_driver)

add_executable(coverage_driver coverage_driver.cpp)

target_include_directories(coverage_driver PRIVATE ${Tpl_INCLUDE_DIRS})
target_link_libraries(coverage_driver PUBLIC
  coverage
  ${Tpl_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

if(ENABLE_COVERAGE_DRIVER)
  add_custom_target(coverage_driver_run
      COMMAND coverage_driver
      "${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests"
      "${EXPERIMENT_HJSON_BUILD_DIR}/src"
      "${CMAKE_BINARY_DIR}/experiments/hjson/tests/player"
      "${CMAKE_BINARY_DIR}/experiments/hjson/tests/scripts/script_4178.lua"
      "${CMAKE_BINARY_DIR}/bin"

      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/experiments/hjson/tests/player"
      DEPENDS hjson_player
      VERBATIM # Protect arguments to commands
  )
endif()

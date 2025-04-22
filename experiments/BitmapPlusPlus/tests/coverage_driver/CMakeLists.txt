# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_driver)

add_executable(bitmap++_driver coverage_driver.cpp)

target_link_libraries(bitmap++_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  bitmap++_tests
  ${BitmapPlusPlus_LIBRARIES}
)

if(ENABLE_COVERAGE_DRIVER)
  add_custom_target(bitmap++_coverage_driver_run
      COMMAND bitmap++_driver
      "${CMAKE_SOURCE_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_tests"
      "${EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR}/src"
      "${CMAKE_BINARY_DIR}/experiments/BitmapPlusPlus/tests/player"
      "${CMAKE_BINARY_DIR}/bin/bitmap++_player"
      "${CMAKE_BINARY_DIR}/experiments/BitmapPlusPlus/tests/player"

      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_tests"
      DEPENDS bitmap++_player
      VERBATIM # Protect arguments to commands
  )
endif()

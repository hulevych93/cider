# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml_driver)

add_executable(pugixml_driver coverage_driver.cpp)

target_link_libraries(pugixml_driver PUBLIC
  coverage
  harmony
  pugixml_tests
)

if(ENABLE_COVERAGE_DRIVER)
  add_custom_target(pugixml_coverage_driver_run
      COMMAND pugixml_driver
      "${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests"
      "${EXPERIMENT_PUGIXML_BUILD_DIR}/src"
      "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/player"
      "${CMAKE_BINARY_DIR}/bin/pugixml_player"
      "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/player"

      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests"
      DEPENDS pugixml_player
      VERBATIM # Protect arguments to commands
  )
endif()

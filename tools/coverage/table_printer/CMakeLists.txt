# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(coverage_table_printer)

add_executable(coverage_table_printer table_printer.cpp)
target_link_libraries(coverage_table_printer PUBLIC
  coverage
  ${PLATFORM_LIBRARIES}
)

if(PRINT_COVERAGE_TABLE)
  add_custom_target(print_coverage_table
      COMMAND coverage_table_printer "${CMAKE_BINARY_DIR}/experiments/hjson/tests/json_reports_clean"

      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      DEPENDS coverage_table_printer
      VERBATIM # Protect arguments to commands
  )
endif()

# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_driver)

add_executable(hjson_driver coverage_driver.cpp)

target_link_libraries(hjson_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  hjson_tests
  q-learning
)

# Helper macro for HJSON run targets
function(add_hjson_run_target target_name mode)
    add_custom_target(${target_name}
        COMMAND hjson_driver
            "${mode}"
            "${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests"
            "${EXPERIMENT_HJSON_BUILD_DIR}/src"
            "${CMAKE_BINARY_DIR}/experiments/hjson/tests/hjson_player"
            "${CMAKE_BINARY_DIR}/bin/hjson_player"
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/results/hjson"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests"
        DEPENDS hjson_player
        VERBATIM
    )
endfunction()

# Define HJSON targets using the macro
add_hjson_run_target(hjson_HS_run 0)
add_hjson_run_target(hjson_CS_run 1)
add_hjson_run_target(hjson_QL_run 2)
add_hjson_run_target(hjson_QLHS_run 3)
add_hjson_run_target(hjson_QLCS_run 4)

# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(argh_driver)

add_executable(argh_driver coverage_driver.cpp)

target_link_libraries(argh_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  argh_tests
  q-learning
  mathplot-log
  ${Python3_LIBRARIES}
)

# Helper macro to define run targets with mode parameter
function(add_argh_run_target target_name mode)
    add_custom_target(${target_name}
        COMMAND argh_driver
            "${mode}"
            "${CMAKE_SOURCE_DIR}/experiments/Argh/tests/argh_tests"
            "${EXPERIMENT_ARGH_BUILD_DIR}"
            "${CMAKE_BINARY_DIR}/experiments/Argh/tests/argh_player"
            "${CMAKE_BINARY_DIR}/bin/argh_player"
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/results/Argh/"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/Argh/tests/argh_tests"
        DEPENDS argh_player
        VERBATIM
    )
endfunction()

add_argh_run_target(argh_HS_run 0)
add_argh_run_target(argh_CS_run 1)
add_argh_run_target(argh_QL_run 2)
add_argh_run_target(argh_QLHS_run 3)
add_argh_run_target(argh_QLCS_run 4)
add_argh_run_target(argh_QGEN_run 5)

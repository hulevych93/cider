# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml_driver)

add_executable(pugixml_driver coverage_driver.cpp)

target_link_libraries(pugixml_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  pugixml_tests
  agent-model
  agent-q-learning
  agent-sarsa-learning
  synthesis
  monte-carlo
  ${PLOT_LIBS}
)

# Helper macro for pugixml run targets with modes
function(add_pugixml_run_target target_name mode)
    add_custom_target(pugixml_${target_name}
        COMMAND pugixml_driver
             "${mode}"
            "${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests"
            "${EXPERIMENT_PUGIXML_BUILD_DIR}/src"
            "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/pugixml_player"
            "${CMAKE_BINARY_DIR}/bin/pugixml_player"
            "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/pugixml_player"
            "${CMAKE_BINARY_DIR}/results/pugixml"
            "${CMAKE_BINARY_DIR}/results/"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests"
        DEPENDS pugixml_player
        VERBATIM
    )
endfunction()

add_pugixml_run_target(HS 0)
add_pugixml_run_target(CS 10)
add_pugixml_run_target(RAND 20)

add_pugixml_run_target(QL 50)

add_pugixml_run_target(QLG1 60)
add_pugixml_run_target(QLG2 61)
add_pugixml_run_target(QLG3 62)

add_pugixml_run_target(QLB1 70)
add_pugixml_run_target(QLB2 71)
add_pugixml_run_target(QLB3 72)

add_pugixml_run_target(CBOX 90)
add_pugixml_run_target(CSTEP 100)
add_pugixml_run_target(LBAR 110)

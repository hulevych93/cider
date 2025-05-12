# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml_driver)

add_executable(pugixml_driver coverage_driver.cpp)

target_link_libraries(pugixml_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  pugixml_tests
  q-learning
  ${PLOT_LIBS}
)

# Helper macro for pugixml run targets with modes
function(add_pugixml_run_target target_name mode)
    add_custom_target(${target_name}
        COMMAND pugixml_driver
             "${mode}"
            "${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests"
            "${EXPERIMENT_PUGIXML_BUILD_DIR}/src"
            "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/pugixml_player"
            "${CMAKE_BINARY_DIR}/bin/pugixml_player"
            "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/pugixml_player"
            "${CMAKE_BINARY_DIR}/results/pugixml"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests"
        DEPENDS pugixml_player
        VERBATIM
    )
endfunction()

# Define the three mode-specific pugixml targets
add_pugixml_run_target(pugixml_HS_run 0)
add_pugixml_run_target(pugixml_CS_run 1)
add_pugixml_run_target(pugixml_QL_run 2)
add_pugixml_run_target(pugixml_QLHS_run 3)
add_pugixml_run_target(pugixml_QLCS_run 4)
add_pugixml_run_target(pugixml_QLGEN_run 5)

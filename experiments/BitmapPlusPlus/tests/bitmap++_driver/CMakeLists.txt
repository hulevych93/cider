# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_driver)

add_executable(bitmap++_driver coverage_driver.cpp)

target_include_directories(bitmap++_driver PRIVATE ${Python3_INCLUDE_DIRS} ${PyBind_INCLUDE_DIRS})

target_link_libraries(bitmap++_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  bitmap++_tests
  agent-model
  agent-q-learning
  agent-sarsa-learning
  synthesis
  monte-carlo
  ${PLOT_LIBS}
)

# Helper macro to define run targets with mode parameter
function(add_bitmap_run_target target_name mode)
    add_custom_target(bitmap++_${target_name}
        COMMAND bitmap++_driver
            "${mode}"
            "${CMAKE_SOURCE_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_tests"
            "${EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR}/src"
            "${CMAKE_BINARY_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_player"
            "${CMAKE_BINARY_DIR}/bin/bitmap++_player"
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/results/BitmapPlusPlus/"
            "${CMAKE_BINARY_DIR}/results/"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_tests"
        DEPENDS bitmap++_player
        VERBATIM
    )
endfunction()

add_bitmap_run_target(HS 0)
add_bitmap_run_target(HS1 1)
add_bitmap_run_target(HS2 2)
add_bitmap_run_target(HS3 3)

add_bitmap_run_target(CS 10)
add_bitmap_run_target(RAND 20)

add_bitmap_run_target(MCTS0 30)
add_bitmap_run_target(MCTS1 31)
add_bitmap_run_target(MCTS2 32)

add_bitmap_run_target(QL 50)
add_bitmap_run_target(SARSA 51)

add_bitmap_run_target(QLG1 60)
add_bitmap_run_target(QLG2 61)
add_bitmap_run_target(QLG3 62)

add_bitmap_run_target(SLG1 65)
add_bitmap_run_target(SLG2 66)
add_bitmap_run_target(SLG3 67)

add_bitmap_run_target(QLB1 70)
add_bitmap_run_target(QLB2 71)
add_bitmap_run_target(QLB3 72)

add_bitmap_run_target(SLB1 75)
add_bitmap_run_target(SLB2 76)
add_bitmap_run_target(SLB3 77)

add_bitmap_run_target(CBOX 90)
add_bitmap_run_target(CSTEP 100)
add_bitmap_run_target(LBAR 110)
add_bitmap_run_target(EFT 120)


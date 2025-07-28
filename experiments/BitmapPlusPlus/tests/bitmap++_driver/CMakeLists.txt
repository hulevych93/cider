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

# Helper macro to define run targets with mode parameter
function(add_bitmap_run_target_group target_name mode group)
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
            "${group}"
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

add_bitmap_run_target(MCTS1 30)
add_bitmap_run_target(MCTS2 31)
add_bitmap_run_target(MCTS3 32)

add_bitmap_run_target(QL 50)
add_bitmap_run_target(SARSA 51)

add_bitmap_run_target(QLEG1 60)
add_bitmap_run_target(QLEG2 61)
add_bitmap_run_target(QLEG3 62)

add_bitmap_run_target(SLEG1 65)
add_bitmap_run_target(SLEG2 66)
add_bitmap_run_target(SLEG3 67)

add_bitmap_run_target(QLB1 70)
add_bitmap_run_target(QLB2 71)
add_bitmap_run_target(QLB3 72)

add_bitmap_run_target(SLB1 75)
add_bitmap_run_target(SLB2 76)
add_bitmap_run_target(SLB3 77)

add_bitmap_run_target_group(CBOX_QLEG 90 0)
add_bitmap_run_target_group(CBOX_QLB 90 1)
add_bitmap_run_target_group(CBOX_SLEG 90 2)
add_bitmap_run_target_group(CBOX_SLB 90 3)
add_bitmap_run_target_group(CBOX_MCTS 90 4)

add_bitmap_run_target_group(CSTEP_QLEG 100 0)
add_bitmap_run_target_group(CSTEP_QLB 100 1)
add_bitmap_run_target_group(CSTEP_SLEG 100 2)
add_bitmap_run_target_group(CSTEP_SLB 100 3)
add_bitmap_run_target_group(CSTEP_MCTS 100 4)
add_bitmap_run_target_group(CSTEP_QLEG_VS_QLB 100 5)

add_bitmap_run_target_group(LBAR_QLEG 110 0)
add_bitmap_run_target_group(LBAR_QLB 110 1)
add_bitmap_run_target_group(LBAR_SLEG 110 2)
add_bitmap_run_target_group(LBAR_SLB 110 3)
add_bitmap_run_target_group(LBAR_MCTS 110 4)
add_bitmap_run_target_group(LBAR_QLEG_VS_QLB 110 5)

add_bitmap_run_target(EFT 120)


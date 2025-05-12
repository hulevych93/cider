# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_driver)

add_executable(bitmap++_driver coverage_driver.cpp)

target_link_libraries(bitmap++_driver PUBLIC
  coverage
  metaheuristics
  pipelines
  bitmap++_tests
  q-learning
  ${PLOT_LIBS}
)

# Helper macro to define run targets with mode parameter
function(add_bitmap_run_target target_name mode)
    add_custom_target(${target_name}
        COMMAND bitmap++_driver
            "${mode}"
            "${CMAKE_SOURCE_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_tests"
            "${EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR}/src"
            "${CMAKE_BINARY_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_player"
            "${CMAKE_BINARY_DIR}/bin/bitmap++_player"
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/results/BitmapPlusPlus/"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/experiments/BitmapPlusPlus/tests/bitmap++_tests"
        DEPENDS bitmap++_player
        VERBATIM
    )
endfunction()

add_bitmap_run_target(bitmap++_HS_run 0)
add_bitmap_run_target(bitmap++_CS_run 1)
add_bitmap_run_target(bitmap++_QL_run 2)
add_bitmap_run_target(bitmap++_QLHS_run 3)
add_bitmap_run_target(bitmap++_QLCS_run 4)
add_bitmap_run_target(bitmap++_QGEN_run 5)

# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(TESTED_FILES ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_value.cpp
    ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_encode.cpp
    ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_decode.cpp
    ${EXPERIMENT_HJSON_BUILD_DIR}/src/hjson_parsenumber.cpp)

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

set(SCRIPT_PATH "${CMAKE_BINARY_DIR}/experiments/hjson/tests/script.lua")

add_subdirectory(recorder)
add_subdirectory(player)
add_subdirectory(coverage_driver)
add_subdirectory(hjson_tests)

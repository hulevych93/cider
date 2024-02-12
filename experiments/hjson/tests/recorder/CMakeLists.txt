# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_recorder)

cider_collect_src(LIST_OUT)

add_executable(hjson_recorder ${LIST_OUT})
add_dependencies(hjson_recorder ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(hjson_recorder PRIVATE ${GEN_DIR})
target_link_libraries(hjson_recorder PUBLIC
  hjson_tests
)

add_custom_target(record_hjson_script
                   COMMAND "${CMAKE_CURRENT_BINARY_DIR}/hjson_recorder" ${SCRIPT_PATH}
                   WORKING_DIRECTORY ${CIDER_DIRECTORY}
                   DEPENDS hjson_recorder
                   COMMENT "Recording script files...")

if(HJSON_ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME hjson_recoder_coverage
        EXECUTABLE hjson_recorder
        EXECUTABLE_ARGS "${CMAKE_BINARY_DIR}/experiments/hjson/tests/script.lua"
        BASE_DIRECTORY "${EXPERIMENT_HJSON_BUILD_DIR}/src"
        WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
        EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*"
    )
endif()

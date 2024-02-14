# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_recorder)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(hjson_recorder hjson_recorder.cpp ${TESTED_FILES})
add_dependencies(hjson_recorder ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(hjson_recorder PRIVATE ${GEN_DIR})
target_link_libraries(hjson_recorder PUBLIC
  hjson_tests
  hjson_wrap
  hjson-models
  recorder
)

if(HJSON_ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME hjson_recoder_coverage
        EXECUTABLE hjson_recorder
        BASE_DIRECTORY "${EXPERIMENT_HJSON_BUILD_DIR}/src"
        WORKING_DIR ${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests
        EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*"
        DEPENDENCIES hjson_tests
    )
endif()

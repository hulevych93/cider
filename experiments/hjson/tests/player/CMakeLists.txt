# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_player)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(hjson_player hjson_play.cpp ${TESTED_FILES})

target_link_libraries(hjson_player PUBLIC
  hjson_wrap
  hjson-models
  scripting
  recorder
  coverage
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

if(HJSON_ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME hjson_player_coverage
        EXECUTABLE hjson_player /Users/mykhailo.hulevych/github/cider/build/experiments/hjson/tests/script.lua
        BASE_DIRECTORY "${EXPERIMENT_HJSON_BUILD_DIR}/src"
        WORKING_DIR ${CMAKE_SOURCE_DIR}/experiments/hjson/tests/hjson_tests
        EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*"
        DEPENDENCIES hjson_tests
    )
endif()

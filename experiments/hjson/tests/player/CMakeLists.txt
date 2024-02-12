# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_player)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

add_executable(hjson_player hjson_play.cpp)

target_link_libraries(hjson_player PUBLIC
  hjson_wrap
  hjson-models
  scripting
  recorder
  coverage
  ${Hjson_LIBRARIES}
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

if(HJSON_ENABLE_PLAYER_COVERAGE)
  add_custom_target(coverage_hjson)

  foreach(INDEX RANGE 1 1259 1)
      setup_target_for_coverage_gcovr_json(
            NAME hjson_player_coverage_${INDEX}
            EXECUTABLE hjson_player
            EXECUTABLE_ARGS "${CMAKE_CURRENT_BINARY_DIR}/../scripts/script_${INDEX}.lua"
            BASE_DIRECTORY "${EXPERIMENT_HJSON_BUILD_DIR}/src"
            EXECUTABLE_WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
            EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*")

          add_dependencies(coverage_hjson hjson_player_coverage_${INDEX})
  endforeach()
endif()

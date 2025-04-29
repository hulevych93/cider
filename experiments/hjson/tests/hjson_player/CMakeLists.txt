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

add_coverage_flags(hjson_player)

target_link_libraries(hjson_player PUBLIC
  hjson_wrap
  hjson-models
  scripting
  recorder
  coverage
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

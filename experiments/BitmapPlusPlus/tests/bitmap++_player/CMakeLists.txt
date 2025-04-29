# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_player)

set(GEN_DIR ${CIDER_GEN_DIR}/bitmap++)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(bitmap++_player player.cpp ${TESTED_FILES})

add_coverage_flags(bitmap++_player)

target_link_libraries(bitmap++_player PUBLIC
  bitmap++_wrap
  bitmap++-models
  scripting
  recorder
  coverage
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

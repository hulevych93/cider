# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml_player)

set(GEN_DIR ${CIDER_GEN_DIR}/pugixml)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(pugixml_player player.cpp ${TESTED_FILES})

add_coverage_flags(pugixml_player)

target_link_libraries(pugixml_player PUBLIC
  pugixml_wrap
  pugixml-models
  scripting
  recorder
  coverage
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

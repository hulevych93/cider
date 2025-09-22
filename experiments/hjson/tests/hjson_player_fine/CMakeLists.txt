# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_player_fine)

set(GEN_DIR ${CIDER_GEN_DIR}/hjson)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

set(PLAYER_LIBS
    hjson_wrap
    hjson-models
    scripting
    recorder
    serialization
    coverage
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
    ${TinyLog_LIBRARIES}
)

add_executable(hjson_player_fine hjson_player_fine.cpp)
target_link_libraries(hjson_player_fine PUBLIC ${PLAYER_LIBS} hjson_traced)

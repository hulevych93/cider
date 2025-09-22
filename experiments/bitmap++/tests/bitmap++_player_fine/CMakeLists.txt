# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_fine_player)

set(GEN_DIR ${CIDER_GEN_DIR}/bitmap++)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

set(PLAYER_LIBS
    bitmap++_wrap
    bitmap++-models
    scripting
    serialization
    recorder
    coverage
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
    ${TinyLog_LIBRARIES}
)

add_executable(bitmap++_player_fine fine_player.cpp)
target_link_libraries(bitmap++_player_fine PUBLIC ${PLAYER_LIBS} bitmap++_traced)

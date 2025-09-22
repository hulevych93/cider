# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(bitmap++_player)

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

add_executable(bitmap++_player player.cpp ${TESTED_FILES})
target_link_libraries(bitmap++_player PUBLIC ${PLAYER_LIBS})

add_library(bitmap++_traced STATIC ${TESTED_FILES})
add_coverage_flags(bitmap++_traced)

add_executable(bitmap++_player_cfg player.cpp)
target_link_libraries(bitmap++_player_cfg PUBLIC ${PLAYER_LIBS} bitmap++_traced)

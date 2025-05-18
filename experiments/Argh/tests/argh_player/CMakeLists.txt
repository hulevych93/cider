# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(argh_player)

set(GEN_DIR ${CIDER_GEN_DIR}/argh)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

set(PLAYER_LIBS
    argh_wrap
    argh-models
    scripting
    serialization
    recorder
    coverage
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)

add_executable(argh_player player.cpp ${TESTED_FILES})
target_link_libraries(argh_player PUBLIC ${PLAYER_LIBS})

add_library(argh_traced STATIC ${TESTED_FILES})
add_coverage_flags(argh_traced)

add_executable(argh_player_cfg player.cpp)
target_link_libraries(argh_player_cfg PUBLIC ${PLAYER_LIBS} argh_traced)

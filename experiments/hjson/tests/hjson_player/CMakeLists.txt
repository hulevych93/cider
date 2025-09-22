# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(hjson_player)

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

add_executable(hjson_player hjson_play.cpp ${TESTED_FILES})
target_link_libraries(hjson_player PUBLIC ${PLAYER_LIBS})

add_library(hjson_traced STATIC ${TESTED_FILES})
add_coverage_flags(hjson_traced)

add_executable(hjson_player_cfg hjson_play.cpp)
target_link_libraries(hjson_player_cfg PUBLIC ${PLAYER_LIBS} hjson_traced)

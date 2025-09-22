# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml_player)

set(GEN_DIR ${CIDER_GEN_DIR}/pugixml)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

set(PLAYER_LIBS
    pugixml_wrap
    pugixml-models
    scripting
    recorder
    serialization
    coverage
    pugixml_traced
    ${Lua_LIBRARIES}
    ${PLATFORM_LIBRARIES}
    ${TinyLog_LIBRARIES}
)

add_executable(pugixml_player player.cpp ${TESTED_FILES})
target_link_libraries(pugixml_player PUBLIC ${PLAYER_LIBS})

add_library(pugixml_traced STATIC ${TESTED_FILES})
add_coverage_flags(pugixml_traced)

add_executable(pugixml_player_cfg player.cpp)
target_link_libraries(pugixml_player_cfg PUBLIC ${PLAYER_LIBS} pugixml_traced)

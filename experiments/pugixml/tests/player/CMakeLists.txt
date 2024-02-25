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

target_link_libraries(pugixml_player PUBLIC
  pugixml_wrap
  pugixml-models
  scripting
  recorder
  coverage
  ${Lua_LIBRARIES}
  ${PLATFORM_LIBRARIES}
)

if(PUGIXML_ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME pugixml_player_coverage
        EXECUTABLE pugixml_player
        EXECUTABLE_ARGS "optimized_pugi.lua"
        BASE_DIRECTORY "${EXPERIMENT_PUGIXML_BUILD_DIR}/src"
        WORKING_DIR ${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests
        EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*"
        DEPENDENCIES pugixml_tests
    )
endif()

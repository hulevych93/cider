# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pugixml_recorder)

set(GEN_DIR ${CIDER_GEN_DIR}/pugixml)

include_directories(.
    ${Lua_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GEN_DIR}
)

add_executable(pugixml_recorder pugixml_recorder.cpp ${TESTED_FILES})
add_dependencies(pugixml_recorder ${CIDER_THIRDPARTY_TARGET_NAME})

target_include_directories(pugixml_recorder PRIVATE ${GEN_DIR})
target_link_libraries(pugixml_recorder PUBLIC
  pugixml_tests
  pugixml_wrap
  pugixml-models
  recorder
)

if(PUGIXML_ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME pugixml_recoder_coverage
        EXECUTABLE pugixml_recorder ${SCRIPT_PATH}
        BASE_DIRECTORY "${EXPERIMENT_PUGIXML_BUILD_DIR}/src"
        WORKING_DIR ${CMAKE_SOURCE_DIR}/experiments/pugixml/tests/pugixml_tests
        EXCLUDE "${CIDER_GEN_DIR}/*" "${BASE_DIRECTORY}/*/v1/*" "/Applications/*"
        DEPENDENCIES pugixml_tests
    )
endif()

# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(TESTED_FILES ${EXPERIMENT_PUGIXML_BUILD_DIR}/src/pugixml.cpp)

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

set(SCRIPT_PATH "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/script.lua")

add_subdirectory(pugixml_tests)
add_subdirectory(recorder)
add_subdirectory(player)
add_subdirectory(coverage_driver)

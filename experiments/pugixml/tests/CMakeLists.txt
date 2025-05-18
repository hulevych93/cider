# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(TESTED_FILES ${EXPERIMENT_PUGIXML_BUILD_DIR}/src/pugixml.cpp)

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

add_subdirectory(pugixml_tests)
add_subdirectory(pugixml_player)
add_subdirectory(pugixml_driver)

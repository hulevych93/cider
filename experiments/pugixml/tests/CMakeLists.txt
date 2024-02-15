# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(TESTED_FILES )

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

set(SCRIPT_PATH "${CMAKE_BINARY_DIR}/experiments/pugixml/tests/script.lua")

add_subdirectory(pugixml_tests)

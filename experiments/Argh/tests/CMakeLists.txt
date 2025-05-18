# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(TESTED_FILES ${EXPERIMENT_ARGH_BUILD_DIR}/argh.cpp)

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

add_subdirectory(argh_tests)
add_subdirectory(argh_player)
add_subdirectory(argh_driver)

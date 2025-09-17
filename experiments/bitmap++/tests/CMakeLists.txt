# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

set(TESTED_FILES ${EXPERIMENT_BITMAPPLUSPLUS_BUILD_DIR}/src/BitmapPlusPlus.cpp)

include_directories(${BitmapPlusPlus_INCLUDE_DIRS})

set_source_files_properties(${TESTED_FILES}
                            PROPERTIES GENERATED TRUE)

add_subdirectory(bitmap++_tests)
add_subdirectory(bitmap++_player)
add_subdirectory(bitmap++_player_fine)
add_subdirectory(bitmap++_driver)

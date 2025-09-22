# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(dynamic-slicer)

cider_collect_src(LIST_OUT "tests")

add_library(dynamic-slicer ${LIST_OUT})

target_include_directories(dynamic-slicer PRIVATE ${TinyLog_INCLUDE_DIRS})

add_dependencies(dynamic-slicer ${CIDER_THIRDPARTY_TARGET_NAME})

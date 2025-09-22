# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(greedy-r)

cider_collect_src(LIST_OUT "tests")

add_library(greedy-r ${LIST_OUT})

target_include_directories(greedy-r PRIVATE ${TinyLog_INCLUDE_DIRS})

add_dependencies(greedy-r ${CIDER_THIRDPARTY_TARGET_NAME})

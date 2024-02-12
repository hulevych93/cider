# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(coverage)

cider_collect_src(LIST_OUT "tests")

add_library(coverage ${LIST_OUT})

target_include_directories(coverage PRIVATE ${NLohMann_INCLUDE_DIRS})

add_dependencies(coverage ${CIDER_THIRDPARTY_TARGET_NAME})

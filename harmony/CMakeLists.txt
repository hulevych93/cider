# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(harmony)

cider_collect_src(LIST_OUT "tests")

add_library(harmony ${LIST_OUT})

target_include_directories(harmony PRIVATE ${NLohMann_INCLUDE_DIRS})

add_dependencies(harmony ${CIDER_THIRDPARTY_TARGET_NAME})

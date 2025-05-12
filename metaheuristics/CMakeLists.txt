# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(metaheuristics)

cider_collect_src(LIST_OUT "tests")

add_library(metaheuristics ${LIST_OUT})

target_include_directories(metaheuristics PRIVATE ${NLohMann_INCLUDE_DIRS})

add_dependencies(metaheuristics ${CIDER_THIRDPARTY_TARGET_NAME})

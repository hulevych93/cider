# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(math)

cider_collect_src(LIST_OUT "tests")

add_library(math-stat ${LIST_OUT})

add_dependencies(math-stat ${CIDER_THIRDPARTY_TARGET_NAME})

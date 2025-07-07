# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(q-learning)

cider_collect_src(LIST_OUT "tests")

add_library(q-learning ${LIST_OUT})

add_dependencies(q-learning ${CIDER_THIRDPARTY_TARGET_NAME})

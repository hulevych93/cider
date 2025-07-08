# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(agent-model)

cider_collect_src(LIST_OUT "tests")

add_library(agent-model ${LIST_OUT})

add_dependencies(agent-model ${CIDER_THIRDPARTY_TARGET_NAME})

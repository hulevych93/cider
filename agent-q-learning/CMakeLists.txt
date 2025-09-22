# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(agent-q-learning)

cider_collect_src(LIST_OUT "tests")

add_library(agent-q-learning ${LIST_OUT})

target_include_directories(agent-q-learning PRIVATE ${TinyLog_INCLUDE_DIRS})

add_dependencies(agent-q-learning ${CIDER_THIRDPARTY_TARGET_NAME})

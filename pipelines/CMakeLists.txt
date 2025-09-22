# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(pipelines)

cider_collect_src(LIST_OUT "tests")

add_library(pipelines ${LIST_OUT})

target_include_directories(pipelines PRIVATE ${TinyLog_INCLUDE_DIRS})

add_dependencies(pipelines ${CIDER_THIRDPARTY_TARGET_NAME})

# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(synthesis)

cider_collect_src(LIST_OUT "tests")

add_library(synthesis ${LIST_OUT})

target_include_directories(synthesis PRIVATE ${TinyLog_INCLUDE_DIRS})

add_dependencies(synthesis ${CIDER_THIRDPARTY_TARGET_NAME})

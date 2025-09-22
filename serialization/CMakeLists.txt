# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(serialization)

cider_collect_src(LIST_OUT "tests")

add_library(serialization ${LIST_OUT})

target_include_directories(serialization PRIVATE ${TinyLog_INCLUDE_DIRS})

add_dependencies(serialization ${CIDER_THIRDPARTY_TARGET_NAME})


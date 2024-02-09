# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(scripting)

cider_collect_src(LIST_OUT "tests")

add_library(scripting ${LIST_OUT})

target_include_directories(scripting PRIVATE ${Lua_INCLUDE_DIRS})

add_dependencies(scripting ${CIDER_THIRDPARTY_TARGET_NAME})

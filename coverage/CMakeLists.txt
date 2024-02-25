# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(coverage)

cider_collect_src(LIST_OUT "tests")

add_library(coverage SHARED ${LIST_OUT})

target_include_directories(coverage PRIVATE ${Tpl_INCLUDE_DIRS})
target_include_directories(coverage PRIVATE ${NLohMann_INCLUDE_DIRS})

target_link_libraries(coverage PUBLIC
  ${Tpl_LIBRARIES}
)

add_dependencies(coverage ${CIDER_THIRDPARTY_TARGET_NAME})

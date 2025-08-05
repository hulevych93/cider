# Copyright (C) 2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(mathplot-log)

cider_collect_src(LIST_OUT "tests")

add_library(mathplot-log ${LIST_OUT})

target_include_directories(mathplot-log PRIVATE ${MathPlot_INCLUDE_DIRS})

add_definitions(-DWITHOUT_NUMPY)
add_definitions(-DENABLE_MATHPLOT)

target_include_directories(mathplot-log PRIVATE ${Python3_INCLUDE_DIRS} ${Fmt_INCLUDE_DIRS} ${PyBind_INCLUDE_DIRS})

add_dependencies(mathplot-log ${CIDER_THIRDPARTY_TARGET_NAME})

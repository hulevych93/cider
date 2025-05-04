# Copyright (C) 2022-2024 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(metaheuristics)

cider_collect_src(LIST_OUT "tests")

add_library(metaheuristics ${LIST_OUT})

target_include_directories(metaheuristics PRIVATE ${NLohMann_INCLUDE_DIRS})

if(ENABLE_MATHPLOT_LOG)
    target_include_directories(metaheuristics PRIVATE ${MathPlot_INCLUDE_DIRS})

    add_definitions(-DWITHOUT_NUMPY)
    add_definitions(-DENABLE_MATHPLOT)

    target_include_directories(metaheuristics PRIVATE ${Python3_INCLUDE_DIRS})
endif()

add_dependencies(metaheuristics ${CIDER_THIRDPARTY_TARGET_NAME})

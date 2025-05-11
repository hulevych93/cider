# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(q-learning)

cider_collect_src(LIST_OUT "tests")

add_library(q-learning ${LIST_OUT})

target_include_directories(q-learning PRIVATE ${NLohMann_INCLUDE_DIRS})

if(ENABLE_MATHPLOT_LOG)
    target_include_directories(q-learning PRIVATE ${MathPlot_INCLUDE_DIRS})

    add_definitions(-DWITHOUT_NUMPY)
    add_definitions(-DENABLE_MATHPLOT)

    target_include_directories(q-learning PRIVATE ${Python3_INCLUDE_DIRS})
endif()

add_dependencies(q-learning ${CIDER_THIRDPARTY_TARGET_NAME})
